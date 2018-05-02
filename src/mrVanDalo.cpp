#include "mrVanDalo.hpp"
#include <math.h>

Plugin *plugin;


#define SLUG_NAME "mrVanDalo"

void init(rack::Plugin *p) {
	plugin = p;
	plugin->slug = SLUG_NAME;
#ifdef VERSION
	p->version = TOSTRING(VERSION);
#endif
	p->website = "https://github.com/mrVanDalo/VCVRecorder";

	p->addModel(createModel<Recorder1Widget>(
			    SLUG_NAME,
			    "Recorder Mono",
			    "Recorder Mono",
			    UTILITY_TAG));

	p->addModel(createModel<Recorder2Widget>(
			    SLUG_NAME,
			    "Recorder 2",
			    "Recorder 2",
			    UTILITY_TAG));
	p->addModel(createModel<Recorder4Widget>(
			    SLUG_NAME,
			    "Recorder 4",
			    "Recorder 4",
			    UTILITY_TAG));
	p->addModel(createModel<Recorder8Widget>(
			    SLUG_NAME,
			    "Recorder 8",
			    "Recorder 8",
			    UTILITY_TAG));
}
