Hue Emulator
============

.. highlight:: c++


A framework for emulating Hue smart light devices.

Refer to specifications available at https://developers.meethue.com (free account login required).

Setup
-----

In your application, remember to add bodyparsers for JSON and XML:

   server.setBodyParser(MIME_JSON, bodyToStringParser);
   server.setBodyParser(MIME_XML, bodyToStringParser);

Without these, you'll get empty bodies for incoming requests.

The :sample:`Basic_Alexa` sample application demonstrates use of provided On/Off, Dimmable and Colour device types
with a global callback function.

Ideally you should provide your own custom Hue devices by inheriting from :cpp:class:`Hue::Device`.
Override the :cpp:method:`Device::getAttribute` method and control behaviour with the return value::

-  :cpp:enum:`Status::Error` The request is rejected, no change made
-  :cpp:enum:`Status::Success` The action was performed immediately
-  :cpp:enum:`Status::Pending` The action was accepted but requires further processing, such as
   sending a command via serial link. The provided callback MUST be invoked when this completes.


API
---

.. doxygenclass:: Hue::Bridge
   :members:

.. doxygenclass:: Hue::Device
   :members:
   
.. doxygenclass:: Hue::OnOffDevice

.. doxygenclass:: Hue::DimmableDevice

.. doxygenclass:: Hue::ColourDevice

