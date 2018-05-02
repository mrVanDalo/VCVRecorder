#include "vcvrecorder.hpp"
#include <math.h>

Plugin *plugin;

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = "vcvrecorder";
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/dekstop/penis_dekstop";

	p->addModel(createModel<Recorder2Widget>(
			    "vcvrecorder",
			    "Recorder2",
			    "Recorder 2",
			    UTILITY_TAG));
	// p->addModel(createModel<Recorder8Widget>("dekstop", "Recorder8", "Recorder 8", UTILITY_TAG));
}
