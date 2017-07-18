#include <instrument.hpp>

using namespace instrument;

namespace instrument_extra {

void mod_load() __attribute((constructor, no_instrument_function));

void mod_load()
{
	util::dbg_info("instrumented_dso_demo.so: module initialized");
}


void mod_unload() __attribute((destructor, no_instrument_function));

void mod_unload()
{
	util::dbg_info("instrumented_dso_demo.so: module finalized");
}


void dso_inner(const i8*) __attribute((visibility("hidden"), noreturn));

void dso_inner(const i8 *arg)
{
	throw exception("instrumented_dso_demo.so: shared library exception (%s)", arg);
}


void dso_main(const i8*) __attribute((noreturn));

void dso_main(const i8 *arg)
{
	dso_inner(arg);
}

}

