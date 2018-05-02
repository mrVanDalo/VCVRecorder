#include "vcvrecorder.hpp"
#include <math.h>

Plugin *plugin;


#define GROUP_NAME "mrVanDalo"

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = "vcvrecorder";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/mrVanDalo/VCVRecorder";

	p->addModel(createModel<Recorder1Widget>(
			    GROUP_NAME,
			    "Recorder Mono",
			    "Recorder Mono",
			    UTILITY_TAG));

	p->addModel(createModel<Recorder2Widget>(
			    GROUP_NAME,
			    "Recorder 2",
			    "Recorder 2",
			    UTILITY_TAG));
	p->addModel(createModel<Recorder4Widget>(
                                           GROUP_NAME,
                                           "Recorder 4",
                                           "Recorder 4",
                                           UTILITY_TAG));
	p->addModel(createModel<Recorder8Widget>(
                                           GROUP_NAME,
                                           "Recorder 8",
                                           "Recorder 8",
                                           UTILITY_TAG));
}
