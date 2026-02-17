# qronicle

Offline Qt-based messenger history visualizer:

| Name | Messages | Avatars |
| :--- | :---:         | :---: |
| <a href="https://en.wikipedia.org/wiki/Facebook_Messenger"><img src="./assets/icons/facebook.svg" width="22" align="center"></a> [**Facebook Messenger**](https://en.wikipedia.org/wiki/Facebook_Messenger) | ✅ | ⏳ |
| <a href="https://en.wikipedia.org/wiki/Skype"><img src="./assets/icons/skype.svg" width="22" align="center"></a> [**Skype**](https://en.wikipedia.org/wiki/Skype) | ✅ | ✅ |
| <a href="https://en.wikipedia.org/wiki/Kopete"><img src="./assets/icons/kopete.svg" width="22" align="center"></a> [**Kopete**](https://en.wikipedia.org/wiki/Kopete) | ✅ | ⏳ |
| <a href="https://en.wikipedia.org/wiki/Psi_(instant_messaging_client)"><img src="./assets/icons/psi.png" width="22" align="center"></a> [**Psi**](https://en.wikipedia.org/wiki/Psi_\(instant_messaging_client\)) | ✅ | ✅ |
| <a href="https://en.wikipedia.org/wiki/Trillian_(software)"><img src="./assets/icons/trillian.png" width="22" align="center"></a> [**Trillian**](https://en.wikipedia.org/wiki/Trillian_\(software\)) | ✅ | ⏳ |
| <a href="https://de.wikipedia.org/wiki/WhatsApp#/media/Datei:WhatsApp_Logo_green.svg"><img src="./assets/icons/whatsapp.svg" width="22" align="center"></a> [**WhatsApp**](https://en.wikipedia.org/wiki/WhatsApp) | ✅ | ⏳ |
| <a href="https://upload.wikimedia.org/wikipedia/commons/9/95/Instagram_logo_2022.svg"><img src="./assets/icons/instagram.svg" width="22" align="center"></a> [**Instagram**](https://en.wikipedia.org/wiki/Instagram) | ⏳ | ⏳ |
| <a href="https://download.kde.org/stable/oxygen-icons"><img src="./assets/icons/mail.png" width="22" align="center"></a> [**Maildir**](https://en.wikipedia.org/wiki/Maildir) (for email) | ⏳ | ⏳ |
| <a href="https://avatars.githubusercontent.com/u/1395850?s=200&v=4"><img src="./assets/icons/gammu.png" width="22" align="center"></a> [**Gammu**](https://github.com/gammu/gammu) (for SMS) | ⏳ | ⏳ |
| <a href="https://upload.wikimedia.org/wikipedia/commons/3/3a/AMSN_icon.svg"><img src="./assets/icons/amsn.svg" width="22" align="center"></a>  [**aMSN**](https://en.wikipedia.org/wiki/AMSN) | ✅ | ✅ |
| <a href="https://www.facebook.com/photo/?fbid=554414996712205&set=a.554414953378876"><img src="./assets/icons/knuddels.jpg" width="22" align="center"></a>  [**Knuddels**](https://de.wikipedia.org/wiki/Knuddels) (German Online Chat) | ⏳ | ⏳ |
| <a href="https://invent.kde.org/network/neochat/-/blob/master/icons/300-apps-neochat.png?ref_type=heads"><img src="./assets/icons/neochat.png" width="22" align="center"></a>  [**NeoChat**](https://apps.kde.org/de/neochat/) | ⏳ | ⏳ |

The name qronicle is derived from the English word chronicle.
However, this name was already in use for different programs.
Since this program uses Qt the name qronicle has been chosen.

The program does not connect online to any of those messengers.
It requires local history data in a certain format to process it.
This is intended behavior since your messenger history might be confidential and should remain on your local machine.

## Planned Features

- Handle directories more separately: aMSN should support multiple folders with address books. Currently, only one "myself" entry and all address books are used togher. Handle it a bit more separately. Same for PSI and other messengers.
- Fill filter text inputs with auto completion by adding SQLite views for all available messengers, nicknames etc.
- Combining filters like multiple senders, multiple receivers, multiple protcools etc.
- Allow adding folders and single files. Folders could contain sub folders with messenger ids like my personal folder and import everything automatically.
- Export everything as XML/JSON into folders.
- A custom XML format for a fake messenger qronicle. XML is better for this than JSON since big messages with formatting and comments are allowed.
- Replay mode: Show some kind of realistic messenging considering the message timestamps to replay how it was send in real time.
- Update logo based on [Kish tablet](https://en.wikipedia.org/wiki/Kish_tablet) which is considered one of the oldest known writing objects in human history.

## Customize

### Themes

Support messenger themes: Sound effects (double clicking on messages), smileys, UI style etc. should look like in the original app.
By default, the program could look up the default folders conta
Themes should be stored in the config folder with the messenger ID as folder name and could be fully customized by the user.
For example, I created `~.config/qronicle/themes/trillian/` with the sub folder `plugins` which contains sound effects and icons in the file structure of Trillian 3.1.
It should then be automatically used.
The same for other messengers.
Themese cannot be part of the program itself due to copyright restrictions.

### Aliases

Define aliases in `~/.config/qronicle/qronicle.conf`:

```txt
[Aliases]
123456=My old account
```

### Avatars

You can place any number of images into `~/.config/qronicle/avatars`.
These will be used as avatars if the file name without extension matches the user ID.

### History

You can place any chat history of different app into `~/.config/qronicle/<app name>`:

- `~/.config/qronicle/kopete`.
- `~/.config/qronicle/skype`.
- `~/.config/qronicle/whatsapp`.

to be found automatically.

## Manual Build Ubuntu

See [build.sh](./build.sh).

## Internationalization

Modify the German translation:

```
/usr/lib/qt6/bin/linguist i18n/app_de.ts
```

## Messenger History Formats

### Kopete

- Folders `~/.local/share/kopete/logs/` or `~/.kde/share/apps/kopete/logs/`.
- KDE 4 folder `~/.kde4/share/apps/kopete/logs/`
- Contain directories with files `<protocol>/<account>/<from>.<year and month>.xml`.

Examples:

- `ICQProtocol/192880392/192780216.200902.xml` with the name schema `<source ICQ number>.<year and month>.xml` contains this:

192780216.200902.xml

```xml
<!DOCTYPE Kopete-History>
<kopete-history version="0.9" >
 <head>
  <date month="2" year="2009" />
  <contact contactId="192880392" type="myself" />
  <contact contactId="192780216" />
 </head>
 <msg nick="Peter" in="1" from="192780216" time="24 11:53:42" >🌹</msg>
 <msg nick="Hans" in="0" from="192880392" time="25 16:59:2" >Nächste Woche!</msg>
</kopete-history>
```

- `JabberProtocol/tamino@cdauth-de/cdauth@cdauth-de.200902.xml`:

```xml
<!DOCTYPE Kopete-History>
<kopete-history version="0.9" >
 <head>
  <date month="2" year="2009" />
  <contact contactId="tamino@cdauth.de" type="myself" />
  <contact contactId="cdauth@cdauth.de" />
 </head>
 <msg nick="Baradé" in="0" from="tamino@cdauth.de" time="28 23:47:9" >🌹</msg>
 <msg nick="cdauth" in="1" from="cdauth@cdauth.de" time="28 23:47:41" >Nächste Woche!</msg>
</kopete-history>
```

- [Sound Effects](https://invent.kde.org/network/kopete/-/tree/master/sounds?ref_type=heads)

### Facebook Messenger

- Export and download activities from Facebook.
- Messages: `your_facebook_activity/messages/inbox/<contact>/<message_xx>.json`.
- Files are in sub folders: `audio`, `files`, `gifs`, `photos` and `videos`.
- Example `your_facebook_activity/messages/inbox/hans_10206072096154041/message_1.json`:

```json
{
  "participants": [
    {
      "name": "Hans"
    },
    {
      "name": "Peter"
    }
  ],
  "messages": [
    {
      "sender_name": "Peter",
      "timestamp_ms": 1620998967908,
      "content": "\u00f0\u009f\u008c\u00b9",
      "is_geoblocked_for_viewer": false,
      "is_unsent_image_by_messenger_kid_parent": false
    },
    {
      "sender_name": "Hans",
      "timestamp_ms": 1616613981404,
      "content": "N\u00c3\u00a4chste Woche!",
      "is_geoblocked_for_viewer": false,
      "is_unsent_image_by_messenger_kid_parent": false
    },
  ],
  "title": "Hans",
  "is_still_participant": true,
  "thread_path": "inbox/hans_10206072096154041",
  "magic_words": [

  ]
}
```

### Skype

SQLite database `~/.Skype/[YourSkypeName]/main.db`.

### PSI

- 192880392_at_icq.jabber.fh%2dstralsund.de.history

```
|2008-12-18T13:50:42|1|from|N---|🌹
|2008-12-18T13:54:05|1|to|N---|Nächste Woche!
```

- cdauth_at_cdauth.de.history

```
|2008-12-14T20:50:08|1|to|N---|🌹
|2008-12-14T20:50:39|1|from|N---|Nächste Woche!
```

### Trillian

- `ICQ/Query/192780216.log`

[16:33] Peter: wa machsch
[16:33] Hans: niöx

- `ICQ/Query/192780216.xml`

```xml
<message type="outgoing_privateMessage" time="1143128021" medium="ICQ" to="192780216" from="192880392" from_display="Barade" text="ni%C3%B6x"/>
<message type="incoming_privateMessage" time="1143128033" medium="ICQ" to="192880392" from="192780216" from_display="192780216" text="schade"/>
```

- `ICQ/Query/192780216-assets.xml`

```xml
<asset medium="ICQ" name="192880392" link="D%3A%5CArbeit%5CProgramme%5CTrillian%5Cusers%5Cdefault%5Cbuddyicons%5Cassets%5CICQ%2D240817250%2D1177074180%2Ejpg"/>
```

- [Trillian 3.1](https://www.oldversion.com.de/software/trillian/trillian-3-1/)
- Icons are located in `Trillian 3/stixe/icons/Default/`.
- Sound effects are located in `Trillian 3/stixe/plugins/Tonal-Sounds/`.
- Emotes are located in `Trillian 3/stixe/plugins/TP21Emoticons-16x16/`.

### SMS

SMS can be extracted with a tool like

```
gammu getallsms > sms_backup.txt
```

The text file contains:

```txt
Speicherplatz 1, Ordner „Eingang“, SIM-Speicher, Posteingangsordner
SMS-Nachricht
SMSC-Nummer          : "+4912345"
Gesendet             : Do 19 Dez 2013 16:46:52  +0100
Zeichenkodierung     : Standard-GSM-Alphabet (keine Kompression)
Nummer               : "+496789"
Status               : Gelesen

Nächste Woche!

```

### WhatsApp

WhatsApp allows exporting chats which leads to directories with all sent and received media like `‎STK-20240525-WA0000.webp` and a text file `WhatsApp-Chat mit Peter.txt`:

```txt
19.12.19, 18:57 - Hans: 🌹
19.12.19, 21:27 - Peter: Nächste Woche!
19.12.19, 21:27 - Peter: ‎STK-20240525-WA0000.webp (Datei angehängt)
```

### Instagram

- <https://www.instagram.com/download/request> to export all your messages.
- Formats: JSON or HTML
- Folder `your_instagram_activity/messages/inbox/` contains folders with the other account names `<account name_id>`
- These folders contain `message_1.json`:

```json
{
  "participants": [
    {
      "name": "Hans"
    },
    {
      "name": "Peter"
    }
  ],
  "messages": [
    {
      "sender_name": "Peter",
      "timestamp_ms": 1762181093195,
      "content": "Hey f\u00c3\u00bcr dich",
      "is_geoblocked_for_viewer": false,
      "is_unsent_image_by_messenger_kid_parent": false
    },
    {
      "sender_name": "Peter",
      "timestamp_ms": 1743164942986,
      "content": "url schau mal",
      "share": {
        "link": "url",
        "share_text": "Website: urlDownloads: url bla bla..."
      },
      "is_geoblocked_for_viewer": false,
      "is_unsent_image_by_messenger_kid_parent": false
    },
    {
      "sender_name": "Hans",
      "timestamp_ms": 1740313151090,
      "photos": [
        {
          "uri": "your_instagram_activity/messages/inbox/peter_12345/photos/12345.jpg",
          "creation_timestamp": 1740313150
        }
      ],
      "is_geoblocked_for_viewer": false,
      "is_unsent_image_by_messenger_kid_parent": false
    }
  ]
}
```

- Besides, they contain sub folders `videos`, `photos` and `audio`.

### Psi

Currently, we support only .history files not .xml files.
History is stored in `~/.psi/profiles/default/history/`.
There can be either .history and .xml files.

.history example:

```txt
|2009-01-26T18:34:25|1|to|N---|Hallo!
|2009-01-26T18:42:25|1|from|N---|Nächste Woche!
```

The value `N---` for the nickname is a placeholder since the vCard info is not loaded here.

XML example:

```xml
<history>
    <message time="2026-02-15T10:30:00Z" is_incoming="true">
        <body>Hallo, wie läuft die Entwicklung von Qronicle?</body>
    </message>
    <message time="2026-02-15T10:31:05Z" is_incoming="false">
        <body>Nächste Woche!</body>
    </message>
</history>
```

Important information about contacts and the account are stored in:

- `~/.psi/profiles/default/vcard/*.xml`: Contact information including birth date and avatars using the standard [XEP-0054: vcard-temp](https://xmpp.org/extensions/xep-0054.html).
- `~/.psi/profiles/default/accounts.xml`: Account name.

vcard XML file:

```xml
<vCard version="2.0" prodid="-//HandGen//NONSGML vGen v1.0//EN" xmlns="vcard-temp" >
    <FN>Hans Müller</FN>
    <NICKNAME>hans</NICKNAME>
    <PHOTO>
        <TYPE>image/jpeg</TYPE>
        <BINVAL>Base64 encoded raw image data</BINVAL>
    </PHOTO>
    <BDAY>1989-03-29</BDAY>
    <URL>website</URL>
</vCard>
```

### aMSN

- `~/.amsn` contains a account data but can also contain other accounts in sub folders called by ther email address with underscores instead of dots and without at symbol.
- `~/.amsn/<account>/abook.xml` address book with all contacts with nick names, last login and logout etc. Your own account is stored with the attribute name="myself":

```xml
<contact name="myself">
	<cid>123456</cid>
	<mfn>Hans</mfn>
	<dp_mimetype>image/png</dp_mimetype>
	<profile_resourceid>12345</profile_resourceid>
	<info_lastchange>2012-05-12T17:04:28.003-07:00</info_lastchange>
	<login>my mail address</login>
	<mob>N</mob>
</contact>
```

`~/.amsn/<account>/displaypic/xxxx.png` is your avatar.

Regular contacts look like this:

```xml
<contact name="email address of the contact">
	<cid>123456</cid>
	<contactguid>UUID</contactguid>
	<space_access>0</space_access>
	<group>UUID</group>
      <displaypicfile>complete base name of the avatar file in displaypic/cache</displaypicfile>
	<last_login>11/30/2011 - 23:39:36</last_login>
	<nick>Klaus</nick>
	<mob>N</mob>
	<lists>FL AL</lists>
</contact>
```

- `~/.amsn/<account>/logs` contains log files and folders like `April 2012`.

Log file content:

```txt
|"LRED[Conversation started on |"LTIME123456]
|"LGRA[|"LTIME123456 ] |"LITAHans :|"LCff00ff hi
|"LGRA[|"LTIME123456 ] |"LITAKaus :|"LC000000 hallo
|"LRED[You have closed the window on 30 Nov 2011 23:39:36]
```

Log file lines with file transfers:

```txt
|"LGRA|"LTIME123456 |"LGRE nickname wants to send you the file ''xxx.jpg'' (6K bytes). It will be saved in directory /home/yyy/amsn_received. Do you want to receive this file?
|"LGRA|"LTIME123456 |"LGRE File transfer accepted
|"LGRA|"LTIME123456 |"LGRE File transfer cancelled
```

- `~/.amsn/<account>/displaypic/cache/<contact email address>/<displaypicfile>` are avatars of contacts
- `~/.amsn/<account>/FT/cache` contains file transfer files.

### Knuddels

- You can export your chats "Chat exportieren" which creates a file `chat_history_<your nickname>_<other nick name>_<year><month><day>_<hour><minute><second>.txt`

```txt
(Hans - 12.02.2026 19:29:19): (hello)
(Peter - 12.02.2026 13:00:10): hey
```

The app provides smileys in form of `(hello)`.

### NeoChat

- ` ~/.config/neochatrc` or `~/.cache/KDE/neochat/ ` contain some non-regular .json file written by libQuotient.

### Steam

- Manual export via Steam support
- `~/.local/share/Steam/config/htmlcache/`
- `~/snap/steam/common/.local/share/Steam/config/htmlcache/`
- Sub folder `Local Storage/leveldb/`

### Microsoft Teams

- `~/.config/Microsoft/Microsoft Teams`
- Export via Microsoft Export Website into JSON
- In browser Chromium profile DB `~/.config/chromium/Default/IndexedDB/https_teams.microsoft.com_0.indexeddb.leveldb/`

### Discord

- `~/.config/discord`
- Chromium: `~/.config/chromium/Default/IndexedDB/https_discord.com_0.indexeddb.leveldb/`

## Credits

- [oxygen-icons](https://download.kde.org/stable/oxygen-icons/)
- [skype-emoticons](https://github.com/refactorsaurusrex/skype-emoticons)
