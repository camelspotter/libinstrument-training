#include <instrument.hpp>

#define SHOW_FUNC_ADDR


namespace instrument {

namespace instrument_extra {

/* Call graph node */
class call: virtual public symbol
{
public:

	list<call> *m_called;			/* Called functions */

	call *m_caller;						/* Caller function */


	call(mem_addr_t addr, const i8 *nm, call *caller)
	try:
	symbol(addr, nm),
	m_called(NULL),
	m_caller(caller)
	{
		m_called = new list<call>;
	}
	catch (...) {
		delete[] m_name;
		m_name = NULL;
		m_called = NULL;
		m_caller = NULL;
	}


	call(const call &src)
	try:
	symbol(src),
	m_called(NULL),
	m_caller(src.m_caller)
	{
		m_called = src.m_called->clone();
	}
	catch (...) {
		delete[] m_name;
		m_name = NULL;
		m_called = NULL;
		m_caller = NULL;
	}


	virtual	~call()
	{
		delete m_called;
		m_called = NULL;
		m_caller = NULL;
	}


	virtual call* clone() const
	{
		return new call(*this);
	}


	virtual call& operator=(const call &rval)
	{
		if ( unlikely(this == &rval) ) {
			return *this;
		}

		symbol::operator=(rval);

		*m_called = *rval.m_called;
		m_caller = rval.m_caller;

		return *this;
	}


	virtual call* add_call(mem_addr_t addr, const i8 *nm)
	{
		call *retval = NULL;

		try {
			retval = new call(addr, nm, this);
			m_called->add(retval);
			return retval;
		}
		catch (...) {
			delete retval;
			throw;
		}
	}


	virtual call& print(u32 depth)
	{
		for (u32 i = 0; likely(i < depth); i++) {
			std::cerr << "  ";
		}

		/* Lazy lookup */
		tracer *iface = tracer::interface();
		if ( likely(m_name == NULL && iface != NULL) ) {
			const i8 *nm =
				iface->proc()
						 ->lookup(m_addr);

			if ( likely(nm != NULL) ) {
				m_name = new (std::nothrow) i8[strlen(nm) + 1];
				if ( likely(m_name != NULL) ) {
					strcpy(m_name, nm);
				}
			}
		}

		std::cerr << m_name;

#ifdef SHOW_FUNC_ADDR
		std::cerr << " @ 0x"
							<< std::hex
							<< m_addr;
#endif

		u32 sz = m_called->size();
		if ( unlikely(sz == 0) ) {
			std::cerr << std::endl;
			return const_cast<call&> (*this);
		}

		std::cerr << " {"
							<< std::endl;

		for (u32 i = 0; likely(i < sz); i++) {
			m_called->at(i)
							->print(depth + 1);
		}

		for (u32 i = 0; likely(i < depth); i++) {
			std::cerr << "  ";
		}

		std::cerr << "}"
							<< std::endl
							<< std::endl;

		return const_cast<call&> (*this);
	}
};


/* Thread call graph */
class graph: virtual public object
{
public:

	call *m_current;				/* The current position in the graph */

	call *m_entry;					/* Thread entry call */

	pthread_t m_handle;			/* Thread handle */


	graph():
	m_current(NULL),
	m_entry(NULL),
	m_handle(pthread_self())
	{
	}


	graph(const graph &src):
	m_current(src.m_current),
	m_entry(NULL),
	m_handle(src.m_handle)
	{
		m_entry = src.m_entry->clone();
	}


	virtual	~graph()
	{
		delete m_entry;
		m_current = m_entry = NULL;
	}


	virtual graph* clone() const
	{
		return new graph(*this);
	}


	virtual graph& operator=(const graph &rval)
	{
		if ( unlikely(this == &rval) ) {
			return *this;
		}

		delete m_entry;
		m_entry = NULL;

		m_current = rval.m_current;
		m_entry = rval.m_entry->clone();

		return *this;
	}


	virtual graph& descend(mem_addr_t addr, const i8 *nm)
	{
		if ( unlikely(m_entry == NULL) ) {
			m_entry = new call(addr, nm, NULL);
			m_current = m_entry;
		}

		else {
			m_current = m_current->add_call(addr, nm);
		}

		return *this;
	}


	virtual graph& ascend()
	{
		call *c = m_current->m_caller;
		if ( likely(c != NULL) ) {
			m_current = c;
		}

		return *this;
	}


	virtual graph& print() const
	{
		m_entry->print(1);
		return const_cast<graph&> (*this);
	}
};


/* List of thread call graphs */
list<graph> *graphs = NULL;

void print_all()
{
	std::cerr << std::endl;
	for (u32 i = 0, sz = graphs->size(); likely(i < sz); i++) {
		graph *g = graphs->at(i);

		std::cerr << "thread 0x"
							<< std::hex
							<< g->m_handle
							<< " {"
							<< std::endl;

		g->print();
		std::cerr << "}"
							<< std::endl
							<< std::endl;
	}
}


graph* current()
{
	for (u32 i = 0, sz = graphs->size(); likely(i < sz); i++) {
		graph *g = graphs->at(i);

		if ( unlikely(pthread_equal(g->m_handle, pthread_self()) != 0) ) {
			return g;
		}
	}

	graph *retval = NULL;
	try {
		retval = new graph;
		graphs->add(retval);
		return retval;
	}
	catch (...) {
		delete retval;
		throw;
	}
}


const i8 *modname = "libmod_callgraph.so";

void mod_load() __attribute((constructor, no_instrument_function));

void mod_load()
{
	try {
		graphs = new list<graph>;
		util::dbg_info("%s: module initialized", modname);
	}
	catch (std::exception &x) {
		std::cerr << x;
	}
}


void mod_unload() __attribute((destructor, no_instrument_function));

void mod_unload()
{
	print_all();

	delete graphs;
	graphs = NULL;

	util::dbg_info("%s: module finalized", modname);
}


class testmod
{
public:

	static void mod_enter(void*, void*);

	static void mod_exit(void*, void*);
};


/* Descend the current call graph, add a node for the called function */
void testmod::mod_enter(void *this_fn, void *call_site)
{
	try {
		current()->descend(reinterpret_cast<mem_addr_t> (this_fn), NULL);
	}

	/*
	 * In non instrumented code sections, like the above (all called functions are
	 * not instrumented) you don't need to create stack traces or unwind the
	 * simulated stack
	 */
	catch (std::exception &x) {
		std::cerr << x;
	}
}


/* Ascend the current call graph */
void testmod::mod_exit(void *this_fn, void *call_site)
{
	try {
		current()->ascend();
	}
	catch (std::exception &x) {
		std::cerr << x;
	}
}

}}
