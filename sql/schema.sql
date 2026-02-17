CREATE TABLE IF NOT EXISTS messages (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    filePath TEXT DEFAULT '',
    lineNumber INTEGER DEFAULT 0,
    sender TEXT DEFAULT '',
    senderNick TEXT DEFAULT '',
    receiver TEXT DEFAULT '',
    receiverNick TEXT DEFAULT '',
    message TEXT DEFAULT '',
    messageHtml TEXT DEFAULT '',
    created_at DATETIME DEFAULT NULL,
    messenger TEXT DEFAULT '',
    protocol TEXT DEFAULT '',
    status TEXT DEFAULT '',
    
    UNIQUE(sender, receiver, message, created_at, messenger, protocol) 
    ON CONFLICT IGNORE -- no duplicates please
);

CREATE INDEX IF NOT EXISTS idx_filePath ON messages(filePath);
CREATE INDEX IF NOT EXISTS idx_sender ON messages(sender);
CREATE INDEX IF NOT EXISTS idx_senderNick ON messages(senderNick);
CREATE INDEX IF NOT EXISTS idx_receiver ON messages(receiver);
CREATE INDEX IF NOT EXISTS idx_receiverNick ON messages(receiverNick);
CREATE INDEX IF NOT EXISTS idx_message ON messages(message);
CREATE INDEX IF NOT EXISTS idx_messageHtml ON messages(messageHtml);
CREATE INDEX IF NOT EXISTS idx_created_at ON messages(created_at);
CREATE INDEX IF NOT EXISTS idx_messenger ON messages(messenger);
CREATE INDEX IF NOT EXISTS idx_protocol ON messages(protocol);
CREATE INDEX IF NOT EXISTS idx_status ON messages(status);

CREATE VIEW IF NOT EXISTS view_all_contacts AS
SELECT sender AS contact FROM messages WHERE sender != ''
UNION
SELECT receiver AS contact FROM messages WHERE receiver != '';

CREATE VIEW IF NOT EXISTS view_all_nicks AS
SELECT senderNick AS nick FROM messages WHERE senderNick != ''
UNION
SELECT receiverNick AS nick FROM messages WHERE receiverNick != '';

CREATE VIEW IF NOT EXISTS view_messenger AS
SELECT DISTINCT messenger
FROM messages
WHERE messenger IS NOT NULL AND messenger != '';

CREATE VIEW IF NOT EXISTS view_protocols AS
SELECT DISTINCT protocol
FROM messages
WHERE protocol IS NOT NULL AND protocol != '';
