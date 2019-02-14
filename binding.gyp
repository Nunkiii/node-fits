{
    "targets": [{

	"target_name": "fits",	

	"sources": [
            "fits/fits.cpp",
            "qk/exception.cpp",
            "qk/pngwriter.cpp",
            "qk/jpeg_writer.cpp"],
        
	"conditions": [
	    
	    ["OS=='linux'", {
		"libraries": ["-fPIC",
                              "-lcfitsio",
                              "-lpng",
                              "-ljpeg"
                             ],	
		
		"cflags_cc": [ "-fexceptions",
                               "-frtti"],
		"cflags_cc!": [
		    "-fno-exceptions",
		]
	    }],
	    
	    ["OS=='mac'", {
		"ldflags": ["-lcfitsio","-lpng", "-ljpeg"],
		"cflags_cc": [ "-fexceptions", "-frtti"],
		"cflags_cc!": [
		    "-fno-exceptions",
		],
                "sources": [ "fits/fmemopen.c", "fits/open_memstream.c"],
	        "include_dirs" : ["/opt/local/include/libpng16"]
        	
            }],	    
        ],

	"include_dirs" : [".",
                          "/usr/local/cfitsio/include",
                          "<!(node -e 'require(\"nan\")')"],

	"xcode_settings": {
	    "OTHER_CFLAGS": [
		"-fexceptions"
	    ],
	    "OTHER_CPLUSPLUSFLAGS": [
		"-fexceptions",
		"-frtti" 
	    ],
	    "OTHER_LDFLAGS": ["-L/usr/local/cfitsio/lib",
                              "-lcfitsio",
                              "-L/opt/local/lib",
                              "-lpng",
                              "-ljpeg"
		             ]
	},
	
    },
               ],
    "dependencies": {
  	"nan": "*"
    }	
}

