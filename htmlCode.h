String s =  "<head>\n"
            "<title>Improved Frame</title>\n"

            "<style type='text/css'>\n"

            "textarea {\n"
            "width: 300px;\n"
            "height: 5em;\n"
            "}\n"

            "textarea {\n"
            "  font-size: 150%;\n"
            "}\n"

            "t1 {\n"
            "  font-size: 150%;\n"
            "  font-weight: bold;\n"
            "}\n"

            "</style>\n" 
          

            "<script>\n"
            "function SubmitAll() { \n document.forms[0].submit();\n }"
            "</script>\n"
            "</head>\n"

            "<body>\n"

            "<form id='form1' action='/string/' method='get' target='_self'>\n"
            "<t1>SSID (32 chars)</t1><br><textarea autofocus maxlength = '32' name = 'input1'></textarea><br>\n" 
            "<t1>SSID Password (16 chars)</t1><br><textarea autofocus maxlength = '16' name = 'input2'></textarea><br>\n" 
            "<t1>Image URL - e.g. http://teamblobfish.com/inkplate/gala.png (70 chars)</t1><br><textarea autofocus maxlength = '70' name = 'input3'></textarea><br>\n" 
            "<t1>Refresh (in minutes - 5 chars)</t1><br><textarea autofocus maxlength = '5' name = 'input4'></textarea><br></form>\n" 

            "<input type='button' value='Submit' onClick='SubmitAll()'>\n"
            "\n"
            "</body>\n";
            
