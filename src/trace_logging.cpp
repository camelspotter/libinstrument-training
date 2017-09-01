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

		<< "\t 0. Log exception trace to file"
		<< std::endl
		<< std::endl

		<< "\t 1. Log exception trace to TCP/IP socket"
		<< std::endl
		<< std::endl

		<< "\t 2. Log exception trace to serial TTY"
		<< std::endl
		<< std::endl

		<< "\t 3. Log exception trace to syntax highlighter"
		<< std::endl
		<< std::endl

		<< "\t 4. Log exception trace to low-constrast syntax highlighter"
		<< std::endl
		<< std::endl;
}


string* log_to_file(exception*) __attribute((no_instrument_function));

string* log_to_file(exception *x)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return NULL;
	}

	string *nm = NULL;
	string *retval = NULL;

	try {
		/* Log the trace to a file, with a unique, trace describing name */
		nm = file::unique_id("%e_%p.trace");
		file log(nm->cstring());

		/* Add the IDP header that describes the trace */
		log	.open()
				.header()
				.append("exception: %s\n\n", x->msg());

		iface->trace(log);
		log.append("\n");

		retval = new string(log);

		log	.flush()
				.close();
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

	delete nm;
	return retval;
}


string* log_to_highlighter(bool) __attribute((no_instrument_function));

string* log_to_highlighter(bool is_low_contrast)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return NULL;
	}

	parser *p = NULL;
	try {
		if (is_low_contrast) {
			p = parser::get_default()->clone();
		}
		else {
			p = parser::get_default();
		}

		p->clear();

		if (is_low_contrast) {
			p	->get_style("number")
				->set_attr_enabled(style::BOLD, false)
				 .set_fgcolor(222);

			p	->get_style("keyword")
				->set_attr_enabled(style::BOLD, false)
				 .set_fgcolor(8);

			p	->get_style("scope")
				->set_attr_enabled(style::BOLD, false);

			style *s = p->get_style("file");
			s->set_fgcolor(13);
			s->set_attr_enabled(style::BOLD, true);
		}

		iface->trace(*p);
		p->append("\n");
		string * retval = p->highlight();

		if (is_low_contrast) {
			delete p;
		}

		return retval;
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}

	if (is_low_contrast) {
		delete p;
	}

	return NULL;
}


string* log_to_idp_server(exception*) __attribute((no_instrument_function));

string* log_to_idp_server(exception *x)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return NULL;
	}

	tcp_socket *client = NULL;
	chain<string> *peer_info = NULL;
	try {
		/* Create and configure the IDP client */
		peer_info = util::getenv("INSTRUMENT_PEER");
		if ( likely(peer_info == NULL) ) {
			client = new tcp_socket(NULL);
		}

		else {
			i32 port = g_idp_tcp_port;
			if ( likely(peer_info->size() > 1) ) {
				port = atoi(peer_info->at(1)->cstring());
			}

			client = new tcp_socket(peer_info->at(0)->cstring(), port);
		}

		client->open();
		delete peer_info;
	}
	catch (exception &err) {
		std::cerr	<< err
							<< std::endl
							<< *iface
							<< std::endl;

		delete peer_info;
		return NULL;
	}

	/* Log the trace to a socket connected to an IDP server */
	string *retval = NULL;
	try {
		/* Add the IDP header that describes the trace */
		client->header();
		client->append("exception: %s\n\n", x->msg());

		iface->trace(*client);
		client->append("\n");
		retval = new string(*client);

		client->flush();
		client->close();
	}
	catch (exception &x)	{
		std::cerr << x;
	}
	catch (std::exception &x)	{
		std::cerr << x;
	}

	delete client;
	return retval;
}


string* log_to_stty(exception*) __attribute((no_instrument_function));

string* log_to_stty(exception *x)
{
	tracer *iface = tracer::interface();
	if ( unlikely(iface == NULL) ) {
		return NULL;
	}

	string *retval = NULL;

	try {
		stty tty("/dev/ttyS0", 115200);
		tty	.open()
				.header()
				.append("exception: %s\n\n", x->msg());

		iface->trace(tty);
		tty.append("\n");
		retval = new string(tty);

		tty	.flush()
				.close();
	}
	catch (exception &x) {
		std::cerr << x;
	}
	catch (std::exception &x) {
		std::cerr << x;
	}

	return retval;
}


/* Dynamic shared object entry point */
void dso_main(const i8*);


/* Stack level 3 (lowest) function */
void level4(const i8 *arg, volatile u64 *desc, void (*cb)(double) = NULL)
{
	dso_main(arg);
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

	if (argc != 2 || (selector = atoi(argv[1])) < 0 || selector > 4) {
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
		level1(basename(nm), selector);
	}
	catch (exception &x) {
		string *trace = NULL;

		switch (selector) {
		case 0:
			trace = log_to_file(&x);
			break;

		case 1:
			trace = log_to_idp_server(&x);
			break;

		case 2:
			trace = log_to_stty(&x);
			break;

		case 3:
			trace = log_to_highlighter(false);
			break;

		case 4:
			trace = log_to_highlighter(true);
		}

		if ( likely(trace != NULL) ) {
			std::cerr	<< x
								<< std::endl
								<< *trace;

			delete trace;
		}
	}
	catch (...) {
		util::dbg_warn("Generic throwable caught");
		std::cerr	<< std::endl
							<< *iface
							<< std::endl;
	}

	/*
	 * If an exception isn't traced you must explicitly unwind the simulated call
	 * stack. This call can be omitted if you are sure that method tracer::trace
	 * is called, even if it actually throws an exception. If the trace is indeed
	 * produced, you can again call tracer::unwind, it won't affect the simulated
	 * call stack
	 */
	iface->unwind();

	delete[] nm;
	return EXIT_SUCCESS;
}

#ifdef __cplusplus
}
#endif

}
