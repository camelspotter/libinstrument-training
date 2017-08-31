#include <instrument.hpp>

using namespace instrument;

/* Use case selector */
i32 selector = -1;


namespace instrument_extra {

class aop_interceptor: virtual public object
{
protected:

	/* Protected variables */

	i32 m_scenario;							/**< @brief Case selector */

public:

	/* Friend classes and functions */

	friend std::ostream& operator<<(std::ostream&, const aop_interceptor&);


	/* Constructors, copy constructors and destructor */

	explicit aop_interceptor(i32);

	aop_interceptor(const aop_interceptor&);

	virtual	~aop_interceptor();

	virtual aop_interceptor* clone() const;


	/* Accessor methods */

	virtual i32 scenario();
	
	virtual aop_interceptor& set_scenario(i32);


	/* Operator overloading methods */

	virtual aop_interceptor& operator=(const aop_interceptor&);


	/* Generic methods */

	virtual aop_interceptor& begin(void*, void*);

	virtual aop_interceptor& end(void*, void*);

	virtual aop_interceptor& load_scenario();

	virtual aop_interceptor& usage(i32, i8**);
	
	
	/* Demo/test methods */
	void level1(i32);
	
	void level2(i32);

	void level3(i32);
	
	void level4(i32);
};


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

	aop_interceptor interceptor(selector);
	interceptor.load_scenario();

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

		srand(time(NULL) + selector);
		i32 times = rand() % 9;
		if ( unlikely(times == 0) ) {
			times = 1;
		}

		std::cout << times << std::endl;
		for (i32 i = 0; likely(i < times); i++) {
			interceptor	.level1(i)
									.level2(i)
									.level3(i)
									.level4(i);
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
