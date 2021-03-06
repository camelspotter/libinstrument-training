#include <instrument.hpp>

using namespace instrument;

/* Use case selector */
i32 selector = -1;


namespace instrument_extra {

/* Usage information console message */
void usage(i32 argc, i8 **argv)
{
	std::cout
		<< " Usage: " << basename(argv[0]) << " <case selector>"
		<< std::endl
		<< std::endl

		<< " The case selector can be one of:"
		<< std::endl
		<< std::endl

		<< "\t 0. Use mod_test interceptor"
		<< std::endl
		<< std::endl

		<< "\t 1. Use mod_callgraph interceptor"
		<< std::endl
		<< std::endl

		<< "\t 2. use inline interceptor"
		<< std::endl
		<< std::endl;
}


void begin(void*, void*) __attribute((no_instrument_function));

/* Instrumentation begin callback */
void begin(void *this_fn, void *call_site)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);

		const i8 *nm = iface->proc()->lookup(addr);
		if ( likely(nm != NULL) ) {
			string msg("inline plugin: %s called @ %p", nm, call_site);
			std::cout << msg << std::endl;
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


void end(void*, void*) __attribute((no_instrument_function));

/* Instrumentation end callback */
void end(void *this_fn, void *call_site)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	try {
		mem_addr_t addr = reinterpret_cast<mem_addr_t> (this_fn);

		const i8 *nm = iface->proc()->lookup(addr);
		if ( likely(nm != NULL) ) {
			string msg("inline plugin: %s returns @ %p", nm, call_site);
			std::cout << msg << std::endl;
		}
	}

	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}
}


void load_interceptor() __attribute((no_instrument_function));

void load_interceptor()
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		exit(EXIT_FAILURE);
	}

	try {
		string path;

		switch (selector) {
		case 0:
			path.set(
				"%s/lib/modules/libinstrument/libmod_test.so",
				util::prefix());

			iface->add_plugin(path.cstring(), NULL);
			break;

		case 1:
			path.set(
				"%s/lib/modules/libinstrument/libmod_callgraph.so",
				util::prefix());

			iface->add_plugin(
				path.cstring(),
				"instrument::instrument_extra::testmod");

			break;

		case 2:
			iface->add_plugin(begin, end);
		}
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}
}


void level1(i32);

void level4(i32 i)
{
	if ( unlikely(((i / 4) % 2) == 0) ) {
		level1(--i);
	}
}


void level3(i32 i)
{
	if ( likely((i % 3) == 0) ) {
		return;
	}

	level4(i);
}


void level2(i32 i)
{
	if ( likely(((i / 2) % 2) == 0) ) {
		return;
	}

	level3(i);
	level4(i);
}


void level1(i32 i)
{
	if ( likely((i % 2) == 0) ) {
		return;
	}

	level2(i);
	level3(i);
	level4(i);
}



#ifdef __cplusplus
extern "C" {
#endif

i32 main(i32 argc, i8 **argv)
{
	/*
	 * Parse the command line arguments, store the ones that are related with the
	 * libinstrument library (prefixed with --instrument-) to the runtime
	 * configuration list and remove them from argv
	 */
	util::init(argc, argv);

	if (argc != 2 || (selector = atoi(argv[1])) < 0 || selector > 2) {
		usage(argc, argv);
		exit(EXIT_FAILURE);
	}

	load_interceptor();

	/*
	 * The interface object is ready right after process initialization. It may be
	 * NULL only if the executable and all of the loaded DSO are stripped of
	 * symbols
	 */
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return EXIT_FAILURE;
	}

	/*
	 * Catch and handle an exception. If you need libinstrument to produce the
	 * stack trace for the exception you need to either feed the interface tracer
	 * object to an output stream, or call tracer::trace to get the trace (into
	 * a instrument::string object or any of its subclasses) and process it
	 */
	try {
		/*
		 * Set the thread name for easy identification. This should better be set in
		 * process constructors, in real life programs
		 */
		iface->proc()
				 ->current_thread()
				 ->set_name("main");

		i32 repetitions = 1;
		if ( unlikely(selector == 1) ) {
			srand(time(NULL) + selector);
			repetitions = rand() % 9;

			if ( unlikely(repetitions == 0) ) {
				repetitions = 1;
			}
		}

		std::cout	<< "Random repetitions: "
							<< repetitions
							<< std::endl;

		for (i32 i = 0; likely(i < repetitions); i++) {
			level1(i);
			level2(i);
			level3(i);
			level4(i);
		}
	}
	catch (exception &x) {
	}
	catch (...) {
	}

	/*
	 * If an exception isn't traced you must explicitly unwind the simulated call
	 * stack. This call can be omitted if you are sure that method tracer::trace
	 * is called, even if it actually throws an exception. If the trace is indeed
	 * produced, you can again call tracer::unwind, it won't affect the simulated
	 * call stack
	 */
	iface->unwind();

	return EXIT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

}
