//
// 	unit test extension for max
//	tim place
//	cycling '74
//

#include "oscar.h"
#include "ext_symobject.h"


// class variables
static t_class		*s_testport_class = NULL;
static t_symbol		*ps_db_ready = NULL;


/************************************************************************/

void testport_classinit(void)
{
	t_class *c = class_new("test.port", (method)testport_new, (method)testport_free, sizeof(t_testport), (method)NULL, A_GIMME, 0L);
	
	class_addmethod(c, (method)testport_notify, 	"notify",	A_CANT, 0);
	class_addmethod(c, (method)testport_send,		"send",		A_CANT, 0);
	class_addmethod(c, (method)testport_ping,		"ping",		0);
		
	class_register(_sym_nobox, c);
	s_testport_class = c;
	ps_db_ready = gensym("###MAX_DB_READY###");
}


/************************************************************************/

void* testport_new(t_symbol *s, long argc, t_atom *argv)
{
	t_testport	*u = (t_testport*)object_alloc(s_testport_class);
	t_max_err	err = MAX_ERR_NONE;
	t_atom		a[5];
	long		port_send = atom_getlong(argv);
	long		port_listen = atom_getlong(argv+1);
	
	if (port_send == 0 || port_listen == 0) {
		cpost("test.port not enabled\n");
	}
	else {
		attr_args_process(u, (short)argc, argv);		
		
		atom_setsym(a+0, gensym("127.0.0.1"));
		atom_setlong(a+1, port_send); // 4792
		err = loadextern(gensym("udpsend"), 2, a, &u->u_udpsend);
		if (!err) {
			object_attach_byptr_register(u, u->u_udpsend, _sym_box);
			cpost("test.port sending on %ld \n", port_send);
		}
		else
			cpost("test.port could not be configured (udpsend) \n");
		
		atom_setlong(a+0, port_listen); // 4791
		atom_setsym(a+1, gensym("@send_notifications"));
		atom_setlong(a+2, 1);
		atom_setsym(a+3, gensym("@quiet"));
		atom_setlong(a+4, 1);
		err = loadextern(gensym("udpreceive"), 5, a, &u->u_udpreceive);
		if (!err) {
			object_attach_byptr_register(u, u->u_udpreceive, _sym_box);
			cpost("test.port listening on %ld \n", port_listen);
		}
		else
			cpost("test.port could not be configured (udpreceive) \n");		
	}
	return u;
}


void testport_free(t_testport *u)
{
	if (u->u_udpreceive)
		object_free(u->u_udpreceive);
}


// use this method to send messages from a network/remote to max or to testmaster, for example:
//    max quit
t_max_err testport_notify(t_testport *u, t_symbol *s, t_symbol *msg, void *sender, void *data)
{
	if (sender == u->u_udpreceive) {
		if (msg == _sym_message) {
			t_symobject *so = (t_symobject*)data;
			t_symbol	*mess = so->sym;
			method		m;
			t_max_err	err;
			long		argc = 0;
			t_atom		*argv = NULL;
			
			if (mess && mess->s_name) {
				err = atom_setparse(&argc, &argv, mess->s_name);
				if (!err) {
					mess = atom_getsym(argv);
					if (mess == gensym("/ping")) {
						testport_send(u, gensym("/ping/return"), 0, NULL);
					}
					else if (mess == gensym("/db/ready?")) {
						if (ps_db_ready->s_thing) {
							testport_send(u, gensym("/db/ready"), 0, NULL);
						}
					}
					else if (mess == gensym("/testdb/path?")) {
						t_atom	a;
						char	ret[MAX_PATH_CHARS];
						
						atom_setsym(&a, gensym(g_dbpath));
						snprintf_zero(ret, MAX_PATH_CHARS, "/testdb/path %s", g_dbpath);
						testport_send(u, gensym("/testdb/path"), 1, &a);
					}
					else if (mess && mess->s_thing && !NOGOOD(mess->s_thing) && argc && argv) {
						m = zgetfn(mess->s_thing, atom_getsym(argv+1));
						
						// messages may come in packed as a single string, or a string with args
						// in either case, the first symbol is the name of an object, whose s_thing should be the destination
						//				   the second symbol is the name of the method
						//				   any additional arguments are, naturally, arguments
						
						// udp callback is on a non-main thread, so we use defer_low() to get back onto the main thread
						if (m)
							defer_low(mess->s_thing, m, atom_getsym(argv+1), (short)argc-2, argv+2);
						else if (so->flags && so->thing) {
							long	ac = so->flags;
							t_atom	*av = (t_atom*)so->thing;
							
							m = zgetfn(mess->s_thing, atom_getsym(av));
							// udp callback is on a non-main thread, so we use defer_low() to get back onto the main thread
							if (m)
								defer_low(mess->s_thing, m, atom_getsym(av), (short)ac-1, av+1);
						}
					}
					sysmem_freeptr(argv);
				}
			}
		}
		else if (msg == _sym_free) {
			object_detach_byptr(u, u->u_udpreceive);
			u->u_udpreceive = NULL;
		}
	}
	return MAX_ERR_NONE;
}


t_max_err testport_send(t_testport *u, t_symbol *msg, long argc, t_atom *argv)
{
	if (u->u_udpsend)
		return object_method_typed(u->u_udpsend, msg, argc, argv, NULL);
	else
		return MAX_ERR_GENERIC;
}


void testport_ping(t_testport *u)
{
	testport_send(u, gensym("/ping/return"), 0, NULL);
}
