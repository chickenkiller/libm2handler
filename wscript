# wscript made for Waf v1.6 or greater.

APPNAME = 'libm2handler'
VERSION = '0.5.5'

from waflib import Logs

def options(opt):
	opt.load('compiler_c compiler_cxx')
	opt.add_option('--debug', action='store_true', default=False, help='Enable debug compilation flags')
	opt.add_option('--cross-prefix', default='', dest='crossprefix', type='string', help='Cross compiling prefix (rootfs path)')

def configure(conf):
	conf.env.version = VERSION

	flags = '-O2 -Wall -Werror'
	mode  = 'Release'

	if conf.options.debug:
		mode = 'Debug'
		flags += ' -g'
	else:
		conf.env.append_unique('DEFINES', ['NDEBUG'])
		# -Wno-error=unused-variable is needed when using assert() with NDEBUG 
		flags +=' -Wno-error=unused-variable'

	conf.msg("Compilation mode", mode)
	flags = flags.split()

	conf.load('compiler_c compiler_cxx')
	conf.check_cfg(package='libzmq' , args='--cflags --libs', uselib_store='ZMQ'    )
	conf.check_cfg(package='jansson', args='--cflags --libs', uselib_store='JANSSON')
	if conf.options.crossprefix:
		conf.env.INCLUDES_ZMQ      = [ conf.options.crossprefix+i for i in conf.env.INCLUDES_ZMQ      ]
		conf.env.LIBPATH_ZMQ       = [ conf.options.crossprefix+i for i in conf.env.LIBPATH_ZMQ       ]
		conf.env.STLIBPATH_ZMQ     = [ conf.options.crossprefix+i for i in conf.env.STLIBPATH_ZMQ     ]
		conf.env.INCLUDES_JANSSON  = [ conf.options.crossprefix+i for i in conf.env.INCLUDES_JANSSON  ]
		conf.env.LIBPATH_JANSSON   = [ conf.options.crossprefix+i for i in conf.env.LIBPATH_JANSSON   ]
		conf.env.STLIBPATH_JANSSON = [ conf.options.crossprefix+i for i in conf.env.STLIBPATH_JANSSON ]

	conf.env.append_unique('CFLAGS', flags + ['-std=gnu99'])
	conf.env.append_unique('CXXFLAGS', flags)

	# DSO stuff
	conf.env.append_unique('LIB_ZMQ','stdc++ pthread uuid rt m'.split())

def build(bld):
	bld( source='m2handler.pc.in', target='m2handler.pc')
	srcdir = bld.path.find_dir('src')
	bld.install_files('${PREFIX}/include/m2handler',
			srcdir.ant_glob('**/*.h',excl=['sha1/config.h']),
			cwd = srcdir,
			relative_trick = True
			)
	bld.stlib( target='sha1', source='src/sha1/sha1.c',                       includes='src/sha1')
	bld.stlib( target='bstr', source='src/bstr/bstrlib.c src/bstr/bstraux.c', includes='src/bstr')
	bld.stlib( target='dict', source='src/adt/dict.c',                        includes='src/adt' )
	for type in 'st sh'.split():
		bld(
				features = 'c c%slib' % type,
				name     = 'm2handler_%s' % type,
				target   = 'm2handler',
				source   = bld.path.ant_glob('src/*.c'),
				vnum     = VERSION,
				includes = 'src',
				export_includes = 'src',
				use      = 'bstr dict sha1 ZMQ JANSSON',
				install_path = '${PREFIX}/lib'
				)
	for handler in 'body_toupper null_handler daemon_to_upper fifo_reader ws_handshake ws_variable'.split():
		bld.program(
				target = handler,
				source = 'example/handlers/%s.c' % handler,
				use    = 'm2handler_sh',
				install_path = None
				)
	bld.program(
			target = 'ws_cpp_handshake',
			source = 'example/handlers/ws_handshake.cpp',
			use    = 'm2handler_sh',
			install_path = None
			)
	for test in 'ws_accept framing dict jansson'.split():
		uses = ''
		if test == 'jansson':
			uses = 'JANSSON'
		else:
			uses = 'm2handler_sh'
		bld.program(
				target = 'test/%s' % test,
				source = 'test/%s.c' % test,
				use    = uses,
				install_path = None
				)

