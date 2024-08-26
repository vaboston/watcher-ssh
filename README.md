# watcher-ssh

Configure your env vars, and build with curl

```
sudo apt install libcurl4-openssl-dev
export TELEGRAM_BOT_TOKEN="your-telegram-bot-token"
export TELEGRAM_CHAT_ID="your-telegram-chat-id"
gcc -o watcher watcher.c  -lcurl
./watcher
```
