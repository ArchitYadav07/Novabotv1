import { useState, useEffect, useRef } from 'react'
import { motion, AnimatePresence } from 'framer-motion'
import { db } from './firebase'
import { ref, set, onValue } from 'firebase/database'
import { 
  MessageSquare, ChevronLeft, Bot, 
  ArrowUp, ArrowDown, ArrowLeft, ArrowRight, Square, Joystick,
  ArrowUpLeft, ArrowUpRight, ArrowDownLeft, ArrowDownRight,
  Send, User, Sparkles, Activity, Cpu
} from 'lucide-react'
import './App.css'

function BootupSequence({ onComplete }) {
  const [stage, setStage] = useState(0)
  useEffect(() => {
    const sequence = [() => setStage(1), () => setStage(2), () => setStage(3), () => setTimeout(onComplete, 1000)]
    sequence.forEach((fn, i) => setTimeout(fn, i * 1500))
  }, [])
  return (
    <div className="bootup-container">
      <AnimatePresence>
        {stage === 1 && <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }} className="boot-stage"><div className="scanner-line" /><p className="boot-text">SCANNING SYSTEM...</p></motion.div>}
        {stage === 2 && <motion.div initial={{ opacity: 0 }} animate={{ opacity: 1 }} exit={{ opacity: 0 }} className="boot-stage matrix">{[...Array(5)].map((_, i) => <motion.div key={i} animate={{ x: [-100, 100], opacity: [0, 1, 0] }} transition={{ duration: 0.5, repeat: Infinity, delay: i * 0.1 }} className="data-stream">{Math.random().toString(16).toUpperCase()}</motion.div>)}<p className="boot-text">LOADING NEURAL CORE...</p></motion.div>}
        {stage === 3 && <motion.div initial={{ scale: 0 }} animate={{ scale: 1.5, opacity: 1 }} exit={{ scale: 10, opacity: 0 }} className="boot-stage core"><div className="core-ignition" /><p className="boot-text">NOVA ONLINE</p></motion.div>}
      </AnimatePresence>
    </div>
  )
}

function App() {
  const [screen, setScreen] = useState('bootup')
  const [input, setInput] = useState('')
  const [messages, setMessages] = useState([{ role: 'bot', text: 'Neural Interface Active. How may I assist you?' }])
  const [isLoading, setIsLoading] = useState(false)
  const messagesEndRef = useRef(null)

  const GROQ_API_KEY = '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'

  useEffect(() => { messagesEndRef.current?.scrollIntoView({ behavior: "smooth" }) }, [messages])

  const sendCommand = async (cmd) => {
    await set(ref(db, 'commands'), { command: cmd, timestamp: Date.now() })
  }

  const handleSend = async () => {
    if (!input.trim() || isLoading) return
    const userMsg = { role: 'user', text: input }
    setMessages(prev => [...prev, userMsg])
    setInput('')
    setIsLoading(true)

    try {
      const response = await fetch('https://api.groq.com/openai/v1/chat/completions', {
        method: 'POST',
        headers: { 'Authorization': `Bearer ${GROQ_API_KEY}`, 'Content-Type': 'application/json' },
        body: JSON.stringify({
          model: 'llama-3.1-8b-instant',
          messages: [
            { role: 'system', content: 'You are Nova Bot. Short, professional AI.' },
            ...messages.slice(-6).map(m => ({ role: m.role === 'bot' ? 'assistant' : 'user', content: m.text })),
            { role: 'user', content: input }
          ]
        })
      })
      const data = await response.json()
      const botText = data.choices[0].message.content
      const utterance = new SpeechSynthesisUtterance(botText);
      window.speechSynthesis.speak(utterance);
      setMessages(prev => [...prev, { role: 'bot', text: botText }])
      await sendCommand(`AI: ${input}`)
    } catch (error) { 
      setMessages(prev => [...prev, { role: 'bot', text: 'Connection logic failed.' }])
    } finally { setIsLoading(false) }
  }

  const JogBtn = ({ dir, icon, isStop }) => (
    <motion.button 
      whileTap={{ scale: 0.9 }}
      onMouseDown={() => sendCommand(dir)} 
      onMouseUp={() => sendCommand('STOP')}
      onTouchStart={() => sendCommand(dir)}
      onTouchEnd={() => sendCommand('STOP')}
      className={`jog-btn ${isStop ? 'stop' : ''}`}
    >
      {icon}
    </motion.button>
  )

  return (
    <div className="app-container">
      <div className="glow-bg" />
      <AnimatePresence mode="wait">
        {screen === 'bootup' && <BootupSequence onComplete={() => setScreen('landing')} />}

        {screen === 'landing' && (
          <motion.div key="landing" initial={{ opacity: 0 }} animate={{ opacity: 1 }} className="hero-section">
            <div className="hero-logo">NOVA BOT</div>
            <button onClick={() => setScreen('menu')} className="enter-btn">INITIALIZE SYSTEM</button>
          </motion.div>
        )}

        {screen === 'menu' && (
          <motion.div key="menu" initial={{ opacity: 0 }} animate={{ opacity: 1 }} className="menu-screen">
             <header className="app-header">
              <button onClick={() => setScreen('landing')} className="back-btn"><ChevronLeft /> Exit</button>
            </header>
            <h1 className="menu-title">Select Module</h1>
            <div className="menu-grid">
              <div onClick={() => setScreen('novatalk')} className="menu-card"><MessageSquare color="#00f2ff" /><span>NovaTalk</span></div>
              <div onClick={() => setScreen('jog')} className="menu-card"><Joystick color="#00f2ff" /><span>8-Axis Jog</span></div>
              <div className="menu-card disabled"><Cpu color="#333" /><span>Neural Link</span></div>
              <div className="menu-card disabled"><Activity color="#333" /><span>Health Sync</span></div>
            </div>
          </motion.div>
        )}

        {screen === 'novatalk' && (
          <motion.div key="talk" initial={{ opacity: 0, x: 20 }} animate={{ opacity: 1, x: 0 }} className="app-main">
            <header className="app-header">
              <button onClick={() => setScreen('menu')} className="back-btn"><ChevronLeft /> Back</button>
              <div className="status-badge"><Sparkles size={12}/> AI Active</div>
            </header>
            <div className="pro-chat-wrapper">
              <div className="chat-messages">
                {messages.map((m, i) => (
                  <div key={i} className={`chat-bubble ${m.role}`}>
                    <div className="bubble-icon">{m.role === 'user' ? <User size={14}/> : <Bot size={14}/>}</div>
                    <div className="bubble-text">{m.text}</div>
                  </div>
                ))}
                <div ref={messagesEndRef} />
              </div>
              <div className="pro-input-container">
                <input value={input} onChange={(e) => setInput(e.target.value)} onKeyPress={(e) => e.key === 'Enter' && handleSend()} placeholder="Command Nova..." />
                <button onClick={handleSend} disabled={isLoading}><Send size={18} /></button>
              </div>
            </div>
          </motion.div>
        )}

        {screen === 'jog' && (
          <motion.div key="jog" initial={{ opacity: 0 }} animate={{ opacity: 1 }} className="app-main">
            <header className="app-header"><button onClick={() => setScreen('menu')} className="back-btn"><ChevronLeft /> Menu</button><div className="status-badge">Manual Override</div></header>
            <div className="jog-grid-8way">
               <JogBtn dir="NW" icon={<ArrowUpLeft />} />
               <JogBtn dir="N" icon={<ArrowUp />} />
               <JogBtn dir="NE" icon={<ArrowUpRight />} />
               <JogBtn dir="W" icon={<ArrowLeft />} />
               <JogBtn dir="STOP" icon={<Square />} isStop />
               <JogBtn dir="E" icon={<ArrowRight />} />
               <JogBtn dir="SW" icon={<ArrowDownLeft />} />
               <JogBtn dir="S" icon={<ArrowDown />} />
               <JogBtn dir="SE" icon={<ArrowDownRight />} />
            </div>
          </motion.div>
        )}
      </AnimatePresence>
    </div>
  )
}

export default App
