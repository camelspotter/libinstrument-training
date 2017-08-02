#include <instrument.hpp>
#include <csignal>
#include <sys/wait.h>

using namespace instrument;

/* Use case selector */
int selector = -1;


namespace instrument_extra {

/* Dynamic shared object entry point */
void dso_main(const i8*);


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

		<< "\t 0. Thread stack tracing"
		<< std::endl
		<< std::endl

		<< "\t 1. Process stack trace dump due to signal"
		<< std::endl
		<< std::endl

		<< "\t 2. Fork a process to execute scenario 0 in parallel"
		<< std::endl
		<< std::endl

		<< "\t 3. List all threads and select one to stack trace"
		<< std::endl
		<< std::endl;
}


/*
 * A signal handler that demonstrates how to dump all simulated call stacks to
 * the standard error stream when a fatal signal (such as SIGSEGV) is received
 * or a fatal error has occured. A snapshot of what the process and each of its
 * threads were executing before the process aborted can prove a valuable debug
 * tool at runtime, even for release editions of a system
 */
void sighandler(i32 signo)
{
	if (signo == SIGCHLD) {
		pid_t pid;
		i32 status;

		while ( likely((pid = waitpid(-1, &status, WNOHANG)) > 0) ) {
#if DBG_LEVEL & DBGL_INFO
			/* If the child process terminated normally */
			if ( likely(WIFEXITED(status)) ) {
				i32 code = WEXITSTATUS(status);
				util::dbg_info("child process %d exited with code %d", pid, code);
			}

			/* If the child process terminated ubnormally due to a signal */
			else if ( unlikely(WIFSIGNALED(status)) ) {
				i32 signo = WTERMSIG(status);

				util::dbg_info(
					"child process %d terminated due to signal %d (%s)",
					pid,
					signo,
					strsignal(signo)
				);
			}

			/* If the child process was stopped due to a signal */
			else if ( unlikely(WIFSTOPPED(status)) ) {
				i32 signo = WSTOPSIG(status);

				util::dbg_info(
					"child process %d suspended due to signal %d (%s)",
					pid,
					signo,
					strsignal(signo)
				);
			}

			/* If the child process was made runnable due to a signal */
			else if ( unlikely(WIFCONTINUED(status)) ) {
				util::dbg_info("child process %d is runnable", pid);
			}
#endif
		}

		return;
	}

	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	/*
	 * Explicit thread synchronization is not needed, all instrument::tracer
	 * public methods are thread safe. This is only done to group the output in an
	 * atomic operation
	 */
	util::lock();

	try {
		/* Get the dump in an instrument::string buffer */
		string buf;
		iface->dump(buf);

		util::dbg_warn("caught signal %d (%s)", signo, strsignal(signo));

		std::cout	<< std::endl
							<< "-- Notice! Call stack dump (all threads) follows --"
							<< std::endl
							<< std::endl

							<< buf

							<< std::endl
							<< "-- Notice! Call stack dump end --"
							<< std::endl
							<< std::endl;
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x)	{
		std::cerr << x;
	}

	util::unlock();
}


/* Register handlers for SIGINT and SIGCHLD */
void register_signal_handlers()
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	try {
		struct sigaction action;
		action.sa_handler = sighandler;
		action.sa_flags = SA_RESTART | SA_NOCLDSTOP;
		sigfillset(&action.sa_mask);

		i32 retval = sigaction(SIGINT, &action, NULL);
		if ( unlikely(retval < 0) ) {
			throw exception(
				"failed to register handler for signal %d (%s) (errno %d - %s)",
				SIGINT,
				strsignal(SIGINT),
				errno,
				strerror(errno)
			);
		}

		retval = sigaction(SIGCHLD, &action, NULL);
		if ( unlikely(retval < 0) ) {
			throw exception(
				"failed to register handler for signal %d (%s) (errno %d - %s)",
				SIGCHLD,
				strsignal(SIGCHLD),
				errno,
				strerror(errno)
			);
		}
	}
	catch (exception &x) {
		std::cerr	<< x
							<< std::endl
							<< *iface
							<< std::endl;
	}
}


void level4(const i8 *id, volatile u64 *desc, void (*cb)(double) = NULL)
{
	pid_t pid;
	string buf;

	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return;
	}

	switch (*desc) {
	case 0:
		dso_main(id);
		break;

	case 1:
		std::cout << "Waiting for interrupt (ctrl+c)..."
							<< std::endl;

		sigpause(SIGINT);
		break;

	case 2:
		pid = fork();

		if (pid != 0) {
			util::dbg_info("parent sleeps");
			sigpause(SIGCHLD);
			util::dbg_info("parent exits");
		}

		else {
			util::dbg_info("child sleeps");
			sleep(1);
			execl("./exception_tracing 0", "exception_tracing", NULL);
			util::dbg_info("child exits");
		}

		break;

	case 3:
		break;
	}
}


template <class T>
void level3(T &id, u32 &desc)
{
	u64 arg = desc;
	level4(id.cstring(), &arg);
}


void level2(const i8 *id, u16 desc)
{
	string tmp(id);
	u32 arg = desc;

	level3<string>(tmp, arg);
}


void level1(const i8 *id, u8 desc)
{
	level2(id, desc);
}


void*	level0(void *arg)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return NULL;
	}

	const i8 *nm = NULL;
	try {
		sleep(1);
		nm = util::executable_path();
		level1(nm, selector);
	}
	catch (exception &x) {
		std::cerr << x
							<< std::endl
							<< *iface
							<< std::endl;
	}
	catch (std::exception &x) {
		std::cerr << x
							<< std::endl
							<< *iface
							<< std::endl;
	}

	delete[] nm;
	return NULL;
}


#ifdef __cplusplus
extern "C" {
#endif

i32 main(i32 argc, i8 **argv)
{
	util::init(argc, argv);

	if (argc != 2 || (selector = atoi(argv[1])) < 0 || selector > 3) {
		usage(argc, argv);
		exit(EXIT_FAILURE);
	}

	/* Register handlers for SIGINT and SIGCHLD */
	register_signal_handlers();

	/*
	 * The interface object is ready right after process initialization. It may be
	 * NULL only if the executable and all of the loaded DSO are stripped of
	 * symbols
	 */
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return EXIT_FAILURE;
	}

	/* Create and start a new thread */
	string arg("test_arg_1::<%d", getpid());
	iface->proc()
			 ->fork_thread("thread-1", level0, (void*) arg.cstring());

	/*
	 * Catch and handle an exception. If you need libinstrument to produce the
	 * stack trace for the exception you need to either feed the interface tracer
	 * object to an output stream, or call tracer::trace to get the trace (into an
	 * instrument::string object or any of its subclasses) and process it
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
		level1(basename(nm), selector);
	}
	catch (exception &x) {
		std::cerr << x
							<< std::endl
							<< *iface
							<< std::endl;
	}
	catch (std::exception &x) {
		try {
			string buf;
			iface->trace(buf);
			std::cerr << x
								<< std::endl
								<< buf
								<< std::endl;
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

	thread *t =
		iface->proc()
				 ->get_thread("thread-1");

	iface->proc()
			 ->thread_join(t, NULL);

	delete[] nm;
	return EXIT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

}
