{
    'targets': [
	{
	    "target_name": "fits",	
	    "sources": [ "colormap/colormap_interface.cpp", "fits/fits.cpp", "qk/exception.cpp", "qk/pngwriter.cpp"],
	    'include_dirs' : ["."],
	    'conditions': [
	    	
	    	}],
        	['OS=="mac"', {
		    'ldflags': ['-lcfitsio','-lpng'],
		    'cflags_cc': [ '-fexceptions', '-frtti'],
		    'cflags_cc!': [
			'-fno-exceptions',
		    ]
        		
        		
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

