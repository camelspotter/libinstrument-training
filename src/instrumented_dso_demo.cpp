#include <instrument.hpp>

using namespace instrument;

namespace instrument_extra {

const i8 *name = "instrumented_dso_demo.so";


void mod_load() __attribute((constructor, no_instrument_function));

void mod_load()
{
	util::dbg_info("%s: module initialized", name);
}


void mod_unload() __attribute((destructor, no_instrument_function));

void mod_unload()
{
	util::dbg_info("%s: module finalized", name);
}


void dso_inner(const i8 *arg)
{
	string data(arg);

	if (!data.match("stack_tracing")) {
		throw exception("%s: shared library exception (%s)", name, arg);
	}

	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	string buf;
	iface->trace(buf, pthread_self());
	util::dbg_warn("Thread stack trace follows");
	std::cout	<< std::endl
						<< buf
						<< std::endl;
}


void dso_main(const i8 *arg)
{
	dso_inner(arg);
}

}
