{
    'targets': [
	{
	    "target_name": "fits",	
	    "sources": [ "colormap/colormap_interface.cpp", "fits/fits.cpp", "qk/exception.cpp", "qk/pngwriter.cpp"],
	    'include_dirs' : [".","/usr/local/cfitsio/include"],

	    'xcode_settings': {
		'OTHER_CFLAGS': [
		    '-fexceptions'
		],
		'OTHER_CPLUSPLUSFLAGS': [
		    '-fexceptions',
		    '-frtti' 
		],
		'OTHER_LDFLAGS': ["-L/usr/local/cfitsio/lib", '-lcfitsio', "-L/opt/local/lib", '-lpng'
		]
	    },

	    'conditions': [
	    	
		['OS=="mac"', {
		    'ldflags': ['-lcfitsio','-lpng'],
		    'cflags_cc': [ '-fexceptions', '-frtti'],
		    'cflags_cc!': [
			'-fno-exceptions',
		    ],
                 "sources": [ "fits/fmemopen.c", "fits/open_memstream.c"],
	         'include_dirs' : ["/opt/local/include/libpng16"]
        	    
        	}],
	    	
	    	
		['OS=="linux"', {
		    'ldflags': ['-lcfitsio','-lpng'],
		    'cflags_cc': [ '-fexceptions', '-frtti'],
		    'cflags_cc!': [
			'-fno-exceptions',
		    ]
		}],
            ],
	    
	},
    ],
  }

