# Chronicle

Qt-based GUI program to visualize, import and export history from different messengers:

| Name | Basic Support | Avatars |
| :--- | :---:         | :---: |
| <a href="https://en.wikipedia.org/wiki/Facebook_Messenger"><img src="./assets/icons/facebook.svg" width="22" align="center"></a> [**Facebook Messenger**](https://en.wikipedia.org/wiki/Facebook_Messenger) | ✅ | ⏳ |
| <a href="https://en.wikipedia.org/wiki/Skype"><img src="./assets/icons/skype.svg" width="22" align="center"></a> [**Skype**](https://en.wikipedia.org/wiki/Skype) | ✅ | ✅ |
| <a href="https://en.wikipedia.org/wiki/Kopete"><img src="./assets/icons/kopete.svg" width="22" align="center"></a> [**Kopete**](https://en.wikipedia.org/wiki/Kopete) | ✅ | ⏳ |
| <a href="https://en.wikipedia.org/wiki/Psi_(instant_messaging_client)"><img src="./assets/icons/psi.png" width="22" align="center"></a> [**Psi**](https://en.wikipedia.org/wiki/Psi_\(instant_messaging_client\)) | ⏳ | ⏳ |
| <a href="https://en.wikipedia.org/wiki/Trillian_(software)"><img src="./assets/icons/trillian.png" width="22" align="center"></a> [**Trillian**](https://en.wikipedia.org/wiki/Trillian_\(software\)) | ✅ | ⏳ |
| <a href="https://de.wikipedia.org/wiki/WhatsApp#/media/Datei:WhatsApp_Logo_green.svg"><img src="./assets/icons/whatsapp.svg" width="22" align="center"></a> [**WhatsApp**](https://en.wikipedia.org/wiki/WhatsApp) | ⏳ | ⏳ |
| <a href="https://avatars.githubusercontent.com/u/1395850?s=200&v=4"><img src="./assets/icons/gammu.png" width="22" align="center"></a> [**Gammu**](https://github.com/gammu/gammu) (for SMS) | ⏳ | ⏳ |
| <a href="https://upload.wikimedia.org/wikipedia/commons/3/3a/AMSN_icon.svg"><img src="./assets/icons/amsn.svg" width="22" align="center"></a>  [**aMSN**](https://en.wikipedia.org/wiki/AMSN) | ⏳ | ⏳ |
| <a href="https://www.facebook.com/photo/?fbid=554414996712205&set=a.554414953378876"><img src="./assets/icons/knuddels.jpg" width="22" align="center"></a>  [**Knuddels**](https://de.wikipedia.org/wiki/Knuddels) | ⏳ | ⏳ |

## Manual Build Ubuntu

See [build.sh](./build.sh).

## Customize

### Distinct

Use the option --distinct to ignore duplicated messages.

### Custom Names

Define aliases in `~/.config/Chronicle/Chronicle.conf`:

```txt
[Aliases]
123456=My old account
```

### Custom Avatars

You can place any number of images into `~/.config/Chronicle/avatars`.
These will be used as avatars if the file name without extension matches the user ID.

### Custom Folders

You can place any chat history of different app into `~/.config/Chronicle/<app name>`:

- `~/.config/Chronicle/kopete`.
- `~/.config/Chronicle/skype`.
- `~/.config/Chronicle/whatsapp`.

to be found automatically.

## Messenger History Formats

### Kopete

- Folders `~/.local/share/kopete/logs/` or `~/.kde/share/apps/kopete/logs/`.
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

### aMSN

`~/.amsn/logs/`

### Knuddels

- You can export your chats "Chat exportieren" which creates a file `chat_history_<your nickname>_<other nick name>_<year><month><day>_<hour><minute><second>.txt`

```txt
(Hans - 12.02.2026 19:29:19): (hello)
(Peter - 12.02.2026 13:00:10): hey
```

The app provides smileys in form of `(hello)`.

## Credits

- [oxygen-icons](https://download.kde.org/stable/oxygen-icons/)
- [skype-emoticons](https://github.com/refactorsaurusrex/skype-emoticons)
