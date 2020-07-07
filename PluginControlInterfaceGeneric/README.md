# Generic Controller for GENIVI AudioManager
Full-featured, robust, stable and flexibly configurable controller plugin for the
GENIVI Audio Manager, which serves the needs of several different project topologies.

It is characterized through below conceptual guidelines:

### Topology

 * Elements like sources, sinks and gateways are hosted in routing-side domains

 * Converters are not supported

 * Sources and sinks are (mandatorily) grouped through assignment to a class element

 * Allowed streaming routes are specified for mentioned class elements

 * Expected system topology with domains as well as static sources, sinks and gateways
   shall be pre-configured in configuration-XML file

 * Dynamic sources, sinks and / or gateways can be allowed. Such dynamic elements
   are to be registered at runtime from routing side.

 * The entire topology as hosted in AudioManager database is reflected through
   element instances inside the plugin

### Events

 * User requests from command side, as well as registration requests and change notifications
   from routing side are abstracted as incoming events and immediately converted to trigger objects

 * A trigger object memorizes the details of an event.

 * Trigger objects are queued and processed sequentially, one at a time.

 * While processing a trigger, appropriate action object(s) are created and configured
   according to business logic

### Action Handling

 * Action objects are queued and executed sequentially, one at a time

 * Action objects can have embedded child actions

 * Action objects, while executing, usually launch commands to the routing side and/or notifications to the command side

 * Action execution may be suspended while waiting for completion of an asynchronous operation on routing side.
   On reception of the response it is resumed automatically.

 * Missing responses (acceptable duration is configurable) cause rolling back the current action

 * Error responses from routing side causes the current action to roll back

### Project-specific policies

 * Project-specific business logic is represented as policy rules and given in XML format

 * Action(s) to be executed are selected and configured per event details

 * Event details are determined from trigger type and trigger parameters

 * Action behavior can be fine-tuned through action parameters

 * A family of functions and macros is available to evaluate policy conditions and action parameters


## Repositories
For further information see  
https://github.com/GENIVI/AudioManager

The generic controller is one of the sample plugins for the AudioManager Project and can be found here  
https://github.com/GENIVI/AudioManagerPlugins  in sub-folder PluginControlInterfaceGeneric


In order to build this component, option **`WITH_GENERIC_CONTROLLER`** must be enabled
from parent folder.


## Documentation
A very detailed documentation is provided by doxygen. In order to use this, please compile
the plugins with

    cmake -DWITH_DOCUMENTATION=ON
    make


## Copyright

Copyright (C) 2012, GENIVI Alliance  
Copyright (C) 2012, BMW AG  
Copyright (C) 2017, Advanced Driver Information Technology  

This file is part of GENIVI Project AudioManager.

Author [Christian Linke](christian.linke@bmw.de) BMW 2011-2015  
Author [Naohiro Nishiguchi](nnishiguchi@jp.adit-jv.com) Advanced Driver Information Technology 2015-2017  
Author [Toshiaki Isogai](<tisogai@jp.adit-jv.com) Advanced Driver Information Technology 2015-2017  
Author [Kapildev Patel](kpatel@jp.adit-jv.com) Advanced Driver Information Technology 2015-2019  
Author [Prashant Jain](pjain@jp.adit-jv.com) Advanced Driver Information Technology  
Author [Martin Koch](mkoch@de.adit-jv.com) Advanced Driver Information Technology 2018-2020


## License

Contribution is done under MPL2.0 or MIT License, depending on the location of the code.

(C) Copyright
This Source Code Form is subject to the terms of the MIT Licence

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR
THE USE OR OTHER DEALINGS IN THE SOFTWARE.

