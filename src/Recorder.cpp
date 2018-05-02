#include <atomic>
#include <functional>
#include <thread>

#include <sstream>
#include <string>

#include "mrVanDalo.hpp"
#include "samplerate.h"
#include "../ext/osdialog/osdialog.h"
#include "write_wav.h"
#include "dsp/digital.hpp"
#include "dsp/ringbuffer.hpp"
#include "dsp/frame.hpp"

#define BLOCKSIZE 1024
#define BUFFERSIZE 32*BLOCKSIZE

template <unsigned int ChannelCount>
struct Recorder : Module {
	enum ParamIds {
		RECORD_PARAM,
		NUM_PARAMS
	};
	enum InputIds {
		TRIGGER_INPUT,
		AUDIO1_INPUT,
		NUM_INPUTS = AUDIO1_INPUT + ChannelCount
	};
	enum OutputIds {
		NUM_OUTPUTS
	};
	enum LightIds {
		RECORDING_LIGHT,
		NUM_LIGHTS
	};

	std::string filename;

	unsigned int counter = 0;

	WAV_Writer writer;
	std::atomic_bool isRecording;
	std::atomic_bool isSharp;
	SchmittTrigger resetTrigger;


	std::mutex mutex;
	std::thread thread;
	RingBuffer<Frame<ChannelCount>, BUFFERSIZE> buffer;
	short writeBuffer[ChannelCount*BUFFERSIZE];

	Recorder() : Module(NUM_PARAMS, NUM_INPUTS, NUM_OUTPUTS, NUM_LIGHTS)
	{
		isRecording = false;
		isSharp = false;
	}
	~Recorder();
	void step();
	void clear();
	void startRecording();
	void stopRecording();
	void saveAsDialog();
	void openWAV();
	void closeWAV();
	void recorderRun();
};

template <unsigned int ChannelCount>
Recorder<ChannelCount>::~Recorder() {
	if (isRecording) stopRecording();
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::clear() {
	filename = "";
	counter = 0;
	isSharp = false;
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::startRecording() {
	if (!filename.empty()) {
		openWAV();
		isRecording = true;
		thread = std::thread(&Recorder<ChannelCount>::recorderRun, this);
	}
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::stopRecording() {
	isRecording = false;
	thread.join();
	closeWAV();
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::saveAsDialog() {
	std::string dir = filename.empty() ? "." : extractDirectory(filename);
	char *path = osdialog_file(OSDIALOG_SAVE, dir.c_str(), "Output", NULL);
	if (path) {
		filename = path;
		free(path);
	} else {
		filename = "";
	}
	isSharp = true;
	counter = 0;
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::openWAV() {
	#ifdef v_050_dev
	float gSampleRate = engineGetSampleRate();
	#endif
	if (!filename.empty()) {

		char counterBuffer[10];
		sprintf( counterBuffer, "%04d", counter );
		std::stringstream ss;
		ss << filename << counterBuffer << ".wav";
		std::string outFilename = ss.str();

		fprintf(stdout, "Recording to %s\n", outFilename.c_str());
		int result = Audio_WAV_OpenWriter(
			&writer,
			outFilename.c_str(),
			gSampleRate,
			ChannelCount
			);
		if (result < 0) {
			isRecording = false;
			char msg[100];
			snprintf(msg, sizeof(msg), "Failed to open WAV file, result = %d\n", result);
			osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, msg);
			fprintf(stderr, "%s", msg);
		}
	}
}

template <unsigned int ChannelCount>
void Recorder<ChannelCount>::closeWAV() {
	fprintf(stdout, "Stopping the recording.\n");
	int result = Audio_WAV_CloseWriter(&writer);
	if (result < 0) {
		char msg[100];
		snprintf(msg, sizeof(msg), "Failed to close WAV file, result = %d\n", result);
		osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, msg);
		fprintf(stderr, "%s", msg);
	}
	isRecording = false;
	counter++;
}

// Run in a separate thread
template <unsigned int ChannelCount>
void Recorder<ChannelCount>::recorderRun() {
	#ifdef v_050_dev
	float gSampleRate = engineGetSampleRate();
	#endif
	while (isRecording) {
		// Wake up a few times a second, often enough to never overflow the buffer.
		float sleepTime = (1.0 * BUFFERSIZE / gSampleRate) / 2.0;
		std::this_thread::sleep_for(std::chrono::duration<float>(sleepTime));
		if (buffer.full()) {
			fprintf(
				stderr,
				"Recording buffer overflow. Can't write quickly enough to disk. Current buffer size: %d\n",
				BUFFERSIZE
				);
		}
		// Check if there is data
		int numFrames = buffer.size();
		if (numFrames > 0) {
			// Convert float frames to shorts
			{
				std::lock_guard<std::mutex> lock(mutex); // Lock during conversion
				src_float_to_short_array(
					static_cast<float*>(buffer.data[0].samples),
					writeBuffer,
					ChannelCount*numFrames
					);
				buffer.start = 0;
				buffer.end = 0;
			}

			fprintf(stdout, "Writing %d frames to disk\n", numFrames);
			int result = Audio_WAV_WriteShorts(&writer, writeBuffer, ChannelCount*numFrames);
			if (result < 0) {
				stopRecording();

				char msg[100];
				snprintf(msg, sizeof(msg), "Failed to write WAV file, result = %d\n", result);
				osdialog_message(OSDIALOG_ERROR, OSDIALOG_OK, msg);
				fprintf(stderr, "%s", msg);
			}
		}
	}
}

/* call where data is processed */
template <unsigned int ChannelCount>
void Recorder<ChannelCount>::step() {

	lights[RECORDING_LIGHT].value = isRecording ? 1.0 : 0.0;

	if(isSharp && !isRecording) {
		if( resetTrigger.process(inputs[TRIGGER_INPUT].value) ) {
			startRecording();
		}
	}

	if(isSharp && isRecording) {
		if( resetTrigger.process(inputs[TRIGGER_INPUT].value) ) {
			stopRecording();
		}
	}

	if (isRecording) {
		// Read input samples into recording buffer
		std::lock_guard<std::mutex> lock(mutex);
		if (!buffer.full()) {
			Frame<ChannelCount> f;
			for (unsigned int i = 0; i < ChannelCount; i++) {
				f.samples[i] = inputs[AUDIO1_INPUT + i].value / 5.0;
			}
			buffer.push(f);
		}
	}
}

struct RecordButton : LEDButton {
	using Callback = std::function<void()>;

	Callback onPressCallback;
	SchmittTrigger recordTrigger;

	void onChange(EventChange &e) override {
		if (recordTrigger.process(value)) {
			onPress(e);
		}
	}
	void onPress(EventChange &e) {
		assert (onPressCallback);
		onPressCallback();
	}
};

struct CounterLabel : Label {
	unsigned int * counter;

	void draw(NVGcontext *vg) override {
		char counterBuffer[10];
		sprintf( counterBuffer, "%04d", *counter );
		bndLabel(vg, 0.0, 0.0, box.size.x, box.size.y, -1, counterBuffer );
	}

};


template <unsigned int ChannelCount>
RecorderWidget<ChannelCount>::RecorderWidget() {
	Recorder<ChannelCount> *module = new Recorder<ChannelCount>();
	setModule(module);

	float margin = 11;
	float docMargin = 24; // distance for input/ouput documentation
	float labelHeight = 15;
	float yPos = margin;
	float xPos = margin;

	unsigned int holes = 7;
	unsigned int holeDistance = 15;

	unsigned int xHalf = holes * (holeDistance / 2) - (holeDistance / 2 );

	box.size = Vec(holeDistance * holes, 380);

	{
		Panel *panel = new LightPanel();
		panel->box.size = box.size;
		addChild(panel);
	}


	{
		xPos = xHalf - 22;
		Label *label = new Label();
		label->box.pos = Vec(xPos, yPos);
		label->text = "VCV-Rec";
		addChild(label);
		yPos += labelHeight;
	}

	// record part
	{
		xPos = xHalf + 2;
		yPos += margin;
		ParamWidget *recordButton = createParam<RecordButton>(
			Vec(xPos, yPos),
			module,
			Recorder<ChannelCount>::RECORD_PARAM,
			0.0,
			1.0,
			0.0
			);
		RecordButton *btn = dynamic_cast<RecordButton*>(recordButton);
		Recorder<ChannelCount> *recorder = dynamic_cast<Recorder<ChannelCount>*>(module);

		btn->onPressCallback = [=]()
				       {
					       if (!recorder->isRecording) {
						       recorder->saveAsDialog();
					       }
				       };
		addParam(recordButton);
		addChild(createLight<SmallLight<RedLight> >(
				 Vec(xPos+6, yPos+5),
				 module,
				 Recorder<ChannelCount>::RECORDING_LIGHT
				 ));


		// Counter
		xPos = xHalf - 12;
		yPos += docMargin - 2;
		CounterLabel *label = new CounterLabel();
		label->box.pos = Vec(xPos, yPos);
		label->counter = &module->counter;
		addChild(label);
		yPos += labelHeight;
	}


	// trigger part
	{
		xPos = xHalf;
		yPos += margin;
		addInput(createInput<PJ301MPort>(Vec(xPos, yPos), module, Recorder<ChannelCount>::TRIGGER_INPUT));
		yPos += docMargin;

		xPos = xHalf - 16;
		Label *label = new Label();
		label->box.pos = Vec(xPos, yPos);
		label->text = "Trigger";
		addChild(label);
	}

	// channels part
	{
		yPos += 3 * margin;
		xPos = xHalf - 22;
		Label *label = new Label();
		label->box.pos = Vec(xPos, yPos);
		label->text = "Channels";
		addChild(label);
		yPos += labelHeight;

		yPos += margin;
		xPos = margin;
		for (unsigned int i = 0; i < ChannelCount; i++) {

			addInput(createInput<PJ3410Port>(Vec(xPos, yPos), module, Recorder<ChannelCount>::AUDIO1_INPUT + i));
			Label *label = new Label();
			label->box.pos = Vec(xPos + 4, yPos + 28);
			label->text = stringf("%d", i + 1);
			addChild(label);

			if (i % 2 == 0) {
				xPos += 37 + margin;
			} else {
				xPos = margin;
				yPos += 40 + margin;
			}
		}

	}
}

Recorder1Widget::Recorder1Widget() :
	RecorderWidget<1u>()
{
}

Recorder2Widget::Recorder2Widget() :
	RecorderWidget<2u>()
{
}

Recorder4Widget::Recorder4Widget() :
	RecorderWidget<4u>()
{
}

Recorder8Widget::Recorder8Widget() :
	RecorderWidget<8u>()
{
}
