import { useState, useEffect, useRef } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { db } from './firebase'
import { ref, set, onValue } from 'firebase/database'
import './App.css'

function App() {
  const [time, setTime] = useState(new Date())
  const [input, setInput] = useState('')
  const [messages, setMessages] = useState([{ role: 'bot', text: 'Hello! I am Nova Bot. How can I help you today?' }])
  const [botState, setBotState] = useState({ mode: 'Interaction', status: 'Online' })
  const [isLoading, setIsLoading] = useState(false)
  const messagesEndRef = useRef(null)

  const GROQ_API_KEY = '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'

  useEffect(() => {
    const timer = setInterval(() => setTime(new Date()), 1000)
    const botRef = ref(db, 'botStatus')
    onValue(botRef, (snapshot) => {
      const data = snapshot.val()
      if (data) setBotState(data)
    })
    return () => clearInterval(timer)
  }, [])

  useEffect(() => {
    messagesEndRef.current?.scrollIntoView({ behavior: "smooth" })
  }, [messages])

  const handleSend = async () => {
    if (!input.trim() || isLoading) return

    const userMsg = { role: 'user', text: input }
    setMessages(prev => [...prev, userMsg])
    setInput('')
    setIsLoading(true)

    try {
      const response = await fetch('https://api.groq.com/openai/v1/chat/completions', {
        method: 'POST',
        headers: {
          'Authorization': `Bearer ${GROQ_API_KEY}`,
          'Content-Type': 'application/json'
        },
        body: JSON.stringify({
          model: 'llama3-8b-8192',
          messages: [
            { 
              role: 'system', 
              content: 'You are Nova Bot, a highly advanced robot developed by NovaTech Robo Pvt Ltd. Keep your responses concise, futuristic, and helpful. If a user asks you to do something with your hardware (like lighting up or moving), acknowledge it.' 
            },
            ...messages.slice(-5).map(m => ({ role: m.role === 'bot' ? 'assistant' : 'user', content: m.text })),
            { role: 'user', content: input }
          ]
        })
      })

      const data = await response.json()
      const botText = data.choices[0].message.content
      
      setMessages(prev => [...prev, { role: 'bot', text: botText }])

      // Push command to Firebase for ESP32
      const commandRef = ref(db, 'commands')
      await set(commandRef, {
        lastCommand: input,
        timestamp: Date.now(),
        aiResponse: botText,
        intent: botText.toLowerCase().includes('light') ? 'LIGHT_UP' : 'IDLE'
      })

    } catch (error) {
      console.error('Error talking to Groq:', error)
      setMessages(prev => [...prev, { role: 'bot', text: 'Sorry, my neural link is experiencing interference.' }])
    } finally {
      setIsLoading(false)
    }
  }

  return (
    <div className="app-container">
      <div className="glow-bg" />
      
      <header className="header">
        <div className="status-badge">
           {botState.mode} Mode • {botState.status}
        </div>
      </header>

      <main className="main-visual">
        <div className="chat-container">
          <div className="messages">
            {messages.map((m, i) => (
              <motion.div 
                key={i} 
                initial={{ opacity: 0, y: 10 }}
                animate={{ opacity: 1, y: 0 }}
                className={`msg ${m.role}`}
              >
                {m.text}
              </motion.div>
            ))}
            {isLoading && <div className="msg bot loading">...</div>}
            <div ref={messagesEndRef} />
          </div>
          
          <div className="input-area">
            <input 
              value={input}
              onChange={(e) => setInput(e.target.value)}
              onKeyPress={(e) => e.key === 'Enter' && handleSend()}
              placeholder="Command Nova Bot..."
            />
            <button onClick={handleSend} disabled={isLoading}>Send</button>
          </div>
        </div>
      </main>

      <footer className="footer">
        <div className="time-display" style={{ fontSize: '1.2rem', marginBottom: '8px' }}>
          {time.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' })}
        </div>
        <div className="branding">
          Powered by NovaTech Robo Pvt Ltd
        </div>
      </footer>
    </div>
  )
}

export default App
