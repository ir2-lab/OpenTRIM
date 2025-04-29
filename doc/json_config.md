# The JSON configuration string {#json_config}

All configuration parameters for a simulation are coded in a [JSON](https://www.json.org/json-el.html) formatted string. 

This can be stored in a plain text file (usually with `.json` extension) which can be either loaded in `opentrim-gui` using the **"Open"** button or read by the \ref cliapp "opentrim cli program" with the following command:

    opentrim -f config.json

In C++, one can use the \ref mcconfig class to parse and validate the JSON string.
                        
The easiest way to get started with preparing a JSON config is to generate
a template with 
                                        
    opentrim -t > template.json
                        
This generates a template with all options set to default values.

@note `opentrim` accepts comments in JSON 

The JSON config string has a self-explanatory structure shown bellow.
Click on any of the options to see more information.

Some more example configurations can be found here:
- \subpage 1MeV_H_on_Fe
- \subpage 2MeV_Fe_on_Fe
- \subpage 3MeV_Xe_on_UO2

\include{doc} options.dox.md

\page 1MeV_H_on_Fe 1MeV H on Fe example

JSON config

\include 1MeV_H_on_Fe.json

\page 2MeV_Fe_on_Fe 2MeV Fe on Fe example

JSON config

\include 2MeV_Fe_on_Fe.json

\page 3MeV_Xe_on_UO2 3MeV Xe on UO2 example

JSON config

\include 3MeV_Xe_on_UO2.json





