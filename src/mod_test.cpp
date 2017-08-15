#include <instrument.hpp>

using namespace instrument;

namespace instrument_extra {

const i8 *name = "libmod_test.so";


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


/* Link the instrumentation functions with C-style linking */

#ifdef __cplusplus
extern "C" {
#endif

void mod_enter(void*, void*) __attribute((no_instrument_function));

/* Instrumentation begin callback */
void mod_enter(void *this_fn, void *call_site)
{
	/*
	 * The interface object is ready right after process initialization. It may be
	 * NULL only if the executable and all of the loaded DSO are stripped of
	 * symbols
	 */
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);

		const i8 *sym =
			iface->proc()
					 ->lookup(addr);

		if ( likely(sym != NULL) ) {
			string msg("%s: %s called @ %p\n", name, sym, call_site);
			std::cout << msg;
		}

		else {
#ifdef WITH_UNRESOLVED
			string msg("%s: %p called @ %p\n", name, this_fn, call_site);
			std::cout << msg;
#endif
		}
	}

	/*
	 * In non instrumented code sections, like the above (all called functions are
	 * not instrumented) you don't need to create stack traces or unwind the
	 * simulated stack
	 */
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}
}


void mod_exit(void*, void*) __attribute((no_instrument_function));

/* Instrumentation end callback */
void mod_exit(void *this_fn, void *call_site)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);

		const i8 *sym =
			iface->proc()
					 ->lookup(addr);

		if ( likely(sym != NULL) ) {
			string msg("%s: %s returned @ %p\n", name, sym, call_site);
			std::cout << msg;
		}

		else {
#ifdef WITH_UNRESOLVED
			string msg("%s: %p returned @ %p\n", name, this_fn, call_site);
			std::cout << msg;
#endif
		}
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}
}

#ifdef __cplusplus
}
#endif

}
