import os
import subprocess
from twitchio.ext import commands

chat_process = subprocess.Popen(
    args=[
      '../../../bld/src/chat/chat',
      '--x_setting',
      'setting.sxproto',
      '--model',
      os.environ['BOT_MODEL'],
    ],
    universal_newlines=True,
    bufsize=1,
    stdin=subprocess.PIPE,
    stdout=subprocess.PIPE,
)

class Bot(commands.Bot):
  def __init__(self):
    super().__init__(
        token=os.environ['ACCESS_TOKEN'],
        client_id=os.environ['CLIENT_ID'],
        nick=os.environ['BOT_NICK'],
        prefix=os.environ['BOT_PREFIX'],
        initial_channels=[os.environ['CHANNEL']],
    )

  async def event_ready(self):
      print(f'Ready | {self.nick}')

  async def event_message(self, message):
    if not message.author:
      print(f'{message.timestamp} - [Unknown Author] -  {message.content}')
      return
    if message.author.name == self.nick:
      print(f'ignoring self')
      return

    print(f'{message.timestamp} - {message.author.name} -  {message.content}')
    content = message.content
    if content.startswith('!qq '):
      content = content[4:]
    elif content.startswith(self.nick):
      pass
    elif content.startswith(f'@{self.nick}'):
      pass
    else:
      print('ignored')
      return

    chat_process.stdin.write(f'/puts\n')
    chat_process.stdin.write(f'/puts USER:\n')
    chat_process.stdin.write(f'/puts {message.author.name}: {content}\n')
    chat_process.stdin.write(f'/puts\n')
    chat_process.stdin.write(f'/puts ASSISTANT:\n')
    chat_process.stdin.write(f'/gets 500 {self.nick}:\n')
    #chat_process.stdin.write(f'/gets 500\n')
    chat_process.stdin.flush()
    s = chat_process.stdout.readline()
    chat_process.stdin.write(f'/rollforget 2048\n')
    chat_process.stdin.flush()
    s = s.strip(' \\')
    s = s.replace('\\_', '_')
    if len(s) > 500:
      s = s[:500]
    await message.channel.send(s)

if __name__ == '__main__':
  print(os.environ['BOT_NICK'])
  bot = Bot()
  bot.run()

