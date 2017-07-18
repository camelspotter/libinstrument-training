#include <instrument.hpp>

using namespace instrument;

int selector = -1;


/**
 * @brief Custom external (not in any namespace) user exception
 */
class external_user_exception: virtual public exception
{
public:

	explicit external_user_exception(const i8 *arg):
	exception("external user exception: %s", arg)
	{
	}
};


namespace instrument_extra {

class user_exception: virtual public exception
{
public:

	explicit user_exception(const i8 *arg):
	exception("user exception: %s", arg)
	{
	}
};


void usage(i32 argc, i8 **argv)
{
	std::cout
		<< "Usage: " << basename(argv[0]) << " <case selector>"
		<< std::endl
		<< std::endl
	
		<< "\t 0 -> Tracing an std::exception"
		<< std::endl
						
		<< "\t 1 -> Tracing an instrument::exception"
		<< std::endl
						
		<< "\t 2 -> Tracing a custom instrument_extra::user_exception"
		<< std::endl
						
		<< "\t 3 -> Tracing a custom external exception (external_user_exception)"
		<< std::endl
						
		<< "\t 4 -> Tracing a generic throwable"
		<< std::endl
						
		<< "\t 5 -> Tracing an exception thrown from a dynamic shared object"
		<< std::endl
		<< std::endl
						
		<< "\t For each scenario the exception tracing is executed both in the main "
		<< "and in a forked thread"
		<< std::endl;
}





void level3(const i8 *arg, int flag)
{
	if (flag == 0) {
		throw external_user_exception(arg);
	}

	else if (flag == 1) {
		throw user_exception(arg);
	}

	else {
		throw exception("demo exception (%s)", arg);
	}
}


void level2(const i8 *arg)
{
	level3(arg, selector);
}


void level1(const i8 *arg)
{
	level2(arg);
}


void* level0(void *arg)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return NULL;
	}

	try {
		level1(static_cast<i8*> (arg));
	}
	catch (exception &x) {
		std::cerr << x
							<< "\r\n"
							<< *iface
							<< "\r\n";
	}
	catch (std::exception &x) {
		std::cerr << x
							<< "\r\n"
							<< *iface
							<< "\r\n";
	}

	return arg;
}


#ifdef __cplusplus
extern "C" {
#endif

i32 main(i32 argc, i8 **argv)
{
	util::init(argc, argv);

	if (argc != 2 || (selector = atoi(argv[1])) < 0 || selector > 5) {
		usage(argc, argv);
		exit(EXIT_FAILURE);
	}

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
	const i8 *nm = NULL;
	try {
		/*
		 * Set the thread name for easy identification. This should better be set in
		 * process constructors, in real life programs
		 */
		iface->proc()
				 ->current_thread()
				 ->set_name("main");

		nm = util::executable_path();
		level1(basename(nm));
	}
	catch (exception &x) {
		std::cerr << x
							<< "\r\n"
							<< *iface
							<< "\r\n";
	}
	catch (std::exception &x) {
		try {
			string buf;
			iface->trace(buf);
			std::cerr << x
								<< "\r\n"
								<< buf
								<< "\r\n";
		}

		/*
		 * In non instrumented code sections, like the above (all called functions
		 * are not instrumented) you don't need to create stack traces or unwind the
		 * simulated stack
		 */
		catch (exception &x) {
			std::cerr << x;
		}
		catch (std::exception &x) {
			std::cerr << x;
		}
	}

	/*
	 * If an exception isn't traced you must explicitly unwind the simulated call
	 * stack. This call can be omitted if you are sure that method tracer::trace
	 * is called, even if it actually throws an exception. If the trace is indeed
	 * produced, you can again call tracer::unwind, it won't affect the simulated
	 * call stack
	 */
	iface->unwind();

	iface->proc()
			 ->fork_thread("thread-1", level0, (void*) "test_arg_1");

	thread *t =
		iface->proc()
				 ->get_thread("thread-1");

	iface->proc()
			 ->thread_join(t);

	delete[] nm;
	return EXIT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

}