const messageForm = document.getElementById('message-form');
const messageInput = document.getElementById('message-input');
const chatContainer = document.getElementById('chat-container');
const contextLengthInput = document.getElementById('context-length-input');
const openaiApiUrlInput = document.getElementById('openai-api-url-input');
const openaiApiKeyInput = document.getElementById('openai-api-key-input');
const openaiModelInput = document.getElementById('openai-model-input');
const saveSettingsBtn = document.getElementById('save-settings-btn');
const resetBtn = document.getElementById('reset-btn');

// Tab switching
const tabs = document.querySelectorAll('.tab-button');
const tabContents = document.querySelectorAll('.tab-content');

tabs.forEach(tab => {
  tab.addEventListener('click', () => {
    const target = tab.getAttribute('data-tab');

    tabs.forEach(t => t.classList.remove('active'));
    tab.classList.add('active');

    tabContents.forEach(content => {
      content.classList.remove('active');
      if (content.id === target) {
        content.classList.add('active');
      }
    });
  });
});

function appendMessage(text, isUser) {
  const messageDiv = document.createElement('div');
  messageDiv.className = isUser ? 'message user-message' : 'message assistant-message';
  messageDiv.innerText = text;
  chatContainer.appendChild(messageDiv);
  chatContainer.scrollTop = chatContainer.scrollHeight;
}

messageForm.addEventListener('submit', async event => {
  event.preventDefault();

  const message = messageInput.value;
  if (!message) return;

  messageInput.value = '';
  appendMessage(message, true);

  try {
    const response = await fetch('/chat', {
      method: 'POST',
      body: JSON.stringify({ message }),
      headers: {
        'Content-Type': 'application/json'
      }
    });

    if (response.ok) {
      const data = await response.json();
      if (data.reply) {
        appendMessage(data.reply, false);
      }
    } else {
      console.error('Error fetching chat response');
    }
  } catch (error) {
    console.error('Network error:', error);
  }
});

// Settings & Reset
resetBtn.addEventListener('click', async () => {
  if (!confirm('Are you sure you want to reset the conversation?')) return;

  try {
    const response = await fetch('/reset', { method: 'POST' });
    if (response.ok) {
      chatContainer.innerHTML = '';
      alert('Conversation reset.');
    } else {
      alert('Failed to reset conversation.');
    }
  } catch (error) {
    console.error('Network error:', error);
  }
});

saveSettingsBtn.addEventListener('click', async () => {
  const contextLength = parseInt(contextLengthInput.value);
  if (!contextLength || contextLength <= 0) {
    alert('Please enter a valid context length.');
    return;
  }

  const settings = {
    context_length: contextLength,
    openai_api_url: openaiApiUrlInput.value.trim(),
    openai_api_key: openaiApiKeyInput.value.trim(),
    openai_model: openaiModelInput.value.trim()
  };

  try {
    const response = await fetch('/settings', {
      method: 'POST',
      body: JSON.stringify(settings),
      headers: { 'Content-Type': 'application/json' }
    });

    if (response.ok) {
      chatContainer.innerHTML = ''; // Changing context length resets conversation
      alert('Settings saved. Conversation reset.');
    } else {
      alert('Failed to save settings.');
    }
  } catch (error) {
    console.error('Network error:', error);
  }
});

// Load settings on start
async function loadSettings() {
  try {
    const response = await fetch('/settings');
    if (response.ok) {
      const data = await response.json();
      if (data.context_length) {
        contextLengthInput.value = data.context_length;
      }
      if (data.openai_api_url) {
        openaiApiUrlInput.value = data.openai_api_url;
      }
      if (data.openai_api_key) {
        openaiApiKeyInput.value = data.openai_api_key;
      }
      if (data.openai_model) {
        openaiModelInput.value = data.openai_model;
      }
    }
  } catch (error) {
    console.error('Failed to load settings:', error);
  }
}

loadSettings();
