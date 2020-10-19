Hue Emulator
============

.. highlight:: c++


A framework for emulating Hue smart light devices via the `Hue::Bridge` class.

A real bridge talks to Hue devices via ZigBee, however with Sming you can control anything
you want using the published API.
Refer to specifications available at https://developers.meethue.com (free account login required).

Setup
-----

Refer to the :sample:`Basic_Alexa` sample for details of how to use this library. Here are a few key notes.

A :cpp:class:`HttpServer` object is required to allow the framework to respond to requests.
Note that Gen 3+ Hue Bridges listen on port 80 (standard HTTP), however older versions use port 1901.
This library has only been tested on port 80.

In your application, remember to add bodyparsers for JSON and XML::

   server.setBodyParser(MIME_JSON, bodyToStringParser);
   server.setBodyParser(MIME_XML, bodyToStringParser);

Without these, you'll get empty bodies for incoming requests.

The sample demonstrates use of provided On/Off, Dimmable and Colour device types
with a global callback function.

Ideally you should provide your own custom Hue devices by inheriting from :cpp:class:`Hue::Device`.
This is demonstrated using `MyHueDevice`. The device ID is 666.

API
---

.. doxygenclass:: Hue::Bridge
   :members:

.. doxygenclass:: Hue::Device
   :members:
   
.. doxygenclass:: Hue::OnOffDevice

.. doxygenclass:: Hue::DimmableDevice

.. doxygenclass:: Hue::ColourDevice

.. doxygenenum:: Hue::Status
