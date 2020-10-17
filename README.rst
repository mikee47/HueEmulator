Hue Emulator
============

A framework for emulating Hue smart light devices.

Refer to specifications available at https://developers.meethue.com (free account login required).

Setup
-----

In your application, remember to add bodyparsers for JSON and XML:

   server.setBodyParser(MIME_JSON, bodyToStringParser);
   server.setBodyParser(MIME_XML, bodyToStringParser);

Without these, you'll get empty bodies for incoming requests.


Philips Hue and Alexa
---------------------

Some notes on the oddities and things identified during development.

Discovery
   
   Note in Philips Hue API docs that SERVER must contain "IpBridge".


description.xml

   These fields must be fixed, change them and Alexa ignores our bridge:
   - manufacturer
   - modelName
   
   These can be changed:
   - modelDescription

   We could build the XML data using JSON, then convert it.
   Prefix element attributes with _.
   Can improve JsonObjectStream in several ways:
   - Don't serialise document all at once, but successively parse individual elements.
   - Add XML as serialisation format (
   
   I did this initially but ended with the RapidXML library as it does a much better job.
   It was essential for correct parsing of SOAP messages.


Discovery and number of requests

   If Alexa is happy with the device info, maybe there's only one request.
   We see repeats sometimes, for example the dimmable issue gets three requests
   so perhaps Alexa retries in case it gets a different result. (Madness ?!)
   
   Deleted 3 devices (one of each type), we get:
      1, 2, 3, 1, 2, 2, 3, 3, 3, 3


Device ID (endpoint)

   Not restricted to 8 bits. 0x7fffffff works, 0xffffffff doesn't.
   We only have room for 24-bits in the unique ID, unless we subvert the MAC address;
   safer to keep that as-is.
   
   Endpoints don't have to be sequential.
   
   Haven't tested non-numerical ID values.

   Alex doesn't appear to take any notice of the `uniqueid` value. We can change it on
   registered lights and it still works. It's the endpoint which matters.

   For reliable operation, need to go through the proper setup, which is tedious.
   However, skipping it causes very flaky behaviour, unresponsive devices, etc.
   and kills the other working ones.


Alexa won't rediscover devices if they've been forgotten.

   I fixed this by appending "-01" to the serial portion to make new unique ID values.
   Clearly Alexa remembers the old ones and chooses to ignore them.
   
   These should probably therefore be either random, or incorporate a value which
   gets incremented.


After discovery, Alexa just uses the endpoints.

   Having said all this, just tried deleting devices then re-addding them and it worked fine.

9/10/19 22:08

   Just did it again, delete device, then rediscover. Got it on second attempt.
   Use specific discovery: hue -> bluetooth NO -> V1 bridge -> Discover.
   
   If state returns 'bri' then it treats it as dimmable light, otherwise just on/off.

25/10/19 13:30

   Echo dot won't respond to 'unauthorized user'. Had to change the device serial number _and_  UDN
   so it treated it as a new device. Suspect the dot would require resetting to get it to work otherwise.
   
   UPDATE: Solution is to just accept provided username when in pairing mode.
   
   Having done this, a user is created successfully and we get a lights/ request, but nothing further.
   This was with lights starting at ID 101. Changed to 200 and it worked.
   
   Probably the unique light ID is getting stored.
   
   It's also still trying to access lights using the other user ID, which of course are failing::
   
      SerialNumber: dc4f221c2666
      Key: 2WLEDHardQrI3WHYTHoMcXHgEspsM8ZZRpSKtBQr
      
      SerialNumber: 1c4f221c2666
      Key: 3d0a37a229b9420850feec09b8cb350a
      
      
      ```
      {
        "userinfo": {
          "devicetype": "Echo",
          "username": "3d0a37a229b9420850feec09b8cb350a"
        }
      }
      ```
   
Alexa shows description as 'unknown device'

   Others say 'Royal Philips Electronics smart device'.
   The light type "Dimmable light" isn't recognised. Can use "Extended color light" for
   this and use attributes to indicate no colour support, works fine.

Alexa shows device as 'not responding'

   Running two bridges works fine until one of them is shut off, which kills the other
   one as well. Perhaps waiting half an hour and the Dot might realise it's only one
   bridge, but I suspect it kills the whole Hue device stack. Delete the offending
   devices (all of them) and it springs back into life. Disabling them does
   the same. Stripping it back to one 'dodgy' light and it gives up after a few
   minutes with 'there was a problem' and the others start working again but it
   doesn't take much to mess it up again.


Stripping back device info

   Returning only state and Alexa picks up on/off and colour lights fine,
   but dimmable returned as 'unknown device'. Turns out Alexa just doesn't recognise
   the lamp type, using 'color' and it's fine.


TODO
----

Add device icons
   Does Alexa pick these up? Probably not, as it only appears to support lights.

Enumerator
   Add to core as template class, perhaps only need clone(), reset() and next().
   Provide find() and count() implementations? Keep it simple.
   As an alternative to Vector lists, can define a linked-list base class, `Enumerable`.
   Each contains a `next` pointer so no additional allocation necessary.

Event notification
   So light state updates in Alexa if changed using other means.
   UPnP says a SUBSCRIBE message is needed to do that, but is one sent?


