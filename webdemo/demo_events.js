(function () {
  var DaoInit, DaoQuit;
  var DaoVmSpace_AcquireProcess, DaoVmSpace_GetNS;
  var DaoVmSpace_StdioStream, DaoProcess_Eval, DaoStream_WriteChars;
  var daoVmSpace, daoNamespace, daoProcess, daoStdio;
  var demos = { "HelloWorld" : "io.writeln( \"Hello World!\" )" };
  var demo_dups = { "HelloWorld" : 0 };
  var printed = false;

  window.Module = {};
  window.Module['print'] = function (x) {
    terminal.setValue( terminal.getValue() + x + '\n' );
	printed = true;
  };
  window.Module['printErr'] = function (x) {
    terminal.setValue( terminal.getValue() + x + '\n' );
	printed = true;
  };
  window.Module['display'] = function (x) {
    var img = document.getElementById('canvas_display'); 
	img.src = x;
  };

  jQuery(document).ready(function() {
    DaoInit = Module.cwrap("DaoInit", "number", ["string"]);
    DaoQuit = Module.cwrap("DaoQuit", "number");
    DaoVmSpace_AcquireProcess = Module.cwrap("DaoVmSpace_AcquireProcess", "number", ["number"]);
    DaoVmSpace_GetNS = Module.cwrap("DaoVmSpace_GetNamespace", "number", ["number", "string"]);
    DaoProcess_Eval = Module.cwrap("DaoProcess_Eval", "number", ["number", "number", "string"]);
	DaoVmSpace_StdioStream = Module.cwrap( "DaoVmSpace_StdioStream", "number", ["number"] );
	DaoStream_WriteChars = Module.cwrap( "DaoStream_WriteChars", "number", ["number", "string"] );

   	DaoWebDemo_AddModules = Module.cwrap("DaoWebDemo_AddModules", "number", ["number"]);

    daoVmSpace = DaoInit("");
	daoStdio = DaoVmSpace_StdioStream( daoVmSpace );
	daoProcess = DaoVmSpace_AcquireProcess( daoVmSpace );
	DaoWebDemo_AddModules( daoVmSpace );

	var demo = "hello.dao";
    jQuery.get('/projects.cgi/Dao/doc/tip/demo/' + demo, function(data) {
		demos[ demo ] = data;
		demo_dups[ demo ] = 0;
		editor.setValue( data );
		editor.clearSelection();
		editor.session.setScrollTop(0);
    });

    jQuery("#submit-button").click(function() {
	
	  if( jQuery("#checkResetOutput").is(':checked') ) terminal.setValue( '' );
	  printed = false;

      var now = new Date().toLocaleString();
      daoNamespace = DaoVmSpace_GetNS( daoVmSpace, "Run: " + now );
      DaoProcess_Eval( daoProcess, daoNamespace, editor.getValue() );
	  if( printed == false ) DaoStream_WriteChars( daoStdio, "\n" );
    });


	jQuery('#select-demo').focus(function (){
		var demo = this.value;
		demos[ demo ] = editor.getValue();
	}).change(function() {
		var demo = jQuery(this).val(); 
		var codes = demos[ demo ];
		if( codes != undefined ){
			editor.setValue( codes );
			editor.clearSelection();
			editor.session.setScrollTop(0);
			return;
		}
        jQuery.get('/projects.cgi/Dao/doc/tip/demo/' + demo, function(data) {
			demos[ demo ] = data;
			demo_dups[ demo ] = 0;
			editor.setValue( data );
			editor.clearSelection();
			editor.session.setScrollTop(0);
        });
	});

    jQuery("#duplicate-demo").click(function() {
		var selected = jQuery('#select-demo').find(":selected");
		var item = selected.text();
		var demo = selected.val();
		var codes = editor.getValue();
		var dupid = demo_dups[ demo ] + 1;
		demos[ demo ] = codes;
		demo_dups[ demo ] += 1;
		demo += ' #' + dupid.toString();
		item += ' #' + dupid.toString();
		demos[ demo ] = codes;
		demo_dups[ demo ] = 0;
		jQuery("#select-demo").append('<option value="' + demo + '">' + item + '</option>');
		jQuery('#select-demo').val(demo);
	});


    window.onbeforeunload = function () {
		DaoQuit();
    }
  });
}());
