import React, { useState, useEffect } from 'react';
import { Activity, ShieldAlert, Cpu, HardDrive, BarChart3, Clock } from 'lucide-react';
import { motion, AnimatePresence } from 'framer-motion';
import ForceGraph from './components/ForceGraph';
import './index.css';

const App = () => {
  const [data, setData] = useState({ nodes: [], links: [], clusters: [] });
  const [session, setSession] = useState({ namespace: 'default', samplingRatio: 0.1 });
  const [metrics, setMetrics] = useState({
    edgeOps: 0,
    avgLatency: 0,
    cycleChecks: 0,
    maxNodes: 0,
    stalls: 0
  });
  const [history, setHistory] = useState([]);

  // Polling for real-time telemetry from C++ Vision Data Exporter
  useEffect(() => {
    const pollInterval = setInterval(async () => {
      try {
        const response = await fetch('/vision_data.json');
        if (!response.ok) throw new Error('C++ Engine Offline');
        const newData = await response.json();
        
        // Only update if data has actually changed to prevent React re-renders
        setData(prev => {
          if (JSON.stringify(prev) === JSON.stringify({ nodes: newData.nodes, links: newData.links, clusters: newData.clusters })) {
            return prev;
          }
          return {
            nodes: newData.nodes || [],
            links: newData.links || [],
            clusters: newData.clusters || []
          };
        });

        setSession(prev => {
          if (prev.namespace === newData.namespace && prev.samplingRatio === newData.samplingRatio) return prev;
          return {
            namespace: newData.namespace || 'default',
            samplingRatio: newData.samplingRatio || 0.1
          };
        });
        
        setMetrics(prev => {
           const newMetrics = {
            edgeOps: newData.metrics.edgeOps,
            avgLatency: newData.metrics.avgLatency,
            cycleChecks: newData.metrics.cycleChecks,
            maxNodes: newData.metrics.maxNodes,
            stalls: (newData.nodes || []).filter(n => n.stalled).length
          };
          if (JSON.stringify(prev) === JSON.stringify(newMetrics)) return prev;
          return newMetrics;
        });

        // Update history for sparklines (limit to last 30 points)
        setHistory(prev => [...prev.slice(-29), {
          time: Date.now(),
          latency: newData.metrics.avgLatency,
          ops: newData.metrics.edgeOps
        }]);

      } catch (err) {
        console.warn("Retrying C++ Data Connection...", err.message);
      }
    }, 1000);
    
    return () => clearInterval(pollInterval);
  }, []);

  return (
    <div className="dashboard-container">
      <header className="header">
        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
          <Activity size={24} color="#3b82f6" />
          <h1 style={{ fontSize: '18px', fontWeight: '700', margin: 0 }}>HPX-VISION ENGINE</h1>
          <span className="status-badge status-active">PROD TELEMETRY</span>
        </div>
        <div style={{ color: '#a1a1aa', fontSize: '12px', display: 'flex', gap: '16px' }}>
          <span>NAMESPACE: <span style={{ color: '#3b82f6', fontWeight: 'bold' }}>{session.namespace}</span></span>
          <span>PID: 45291</span>
        </div>
      </header>

      <aside className="sidebar-left">
        <h2 style={{ fontSize: '14px', color: '#71717a', marginBottom: '16px' }}>PERFORMANCE METRICS</h2>
        
        <div className="card">
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#a1a1aa', fontSize: '12px', marginBottom: '8px' }}>
            <Cpu size={14} /> ADAPTIVE SAMPLING
          </div>
          <div className="metric-value">{(session.samplingRatio * 100).toFixed(1)} <span style={{ fontSize: '14px', color: '#71717a' }}>%</span></div>
          <div className="sparkline">
             {history.map((h, i) => (
               <div key={i} className="spark-bar" style={{ height: `${Math.min(100, (h.ops / 10000) * 100)}%` }}></div>
             ))}
          </div>
        </div>

        <div className="card">
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#a1a1aa', fontSize: '12px', marginBottom: '8px' }}>
            <HardDrive size={14} /> TASK OVERHEAD
          </div>
          <div className="metric-value">{metrics.avgLatency} <span style={{ fontSize: '14px', color: '#71717a' }}>ns</span></div>
          <div className="sparkline">
             {history.map((h, i) => (
               <div key={i} className="spark-bar" style={{ height: `${Math.min(100, (h.latency / 2000) * 100)}%`, background: '#3b82f6' }}></div>
             ))}
          </div>
        </div>

        <div className="card">
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#a1a1aa', fontSize: '12px', marginBottom: '8px' }}>
            <Clock size={14} /> FAULT TOLERANCE
          </div>
          <div className="metric-value">TTL-5s <span style={{ fontSize: '14px', color: '#34d399' }}>ACTIVE</span></div>
        </div>
      </aside>

      <main className="main-content">
        <ForceGraph data={data} />
        
        <div style={{ position: 'absolute', bottom: '24px', left: '24px', pointerEvents: 'none' }}>
           <div style={{ color: '#71717a', fontSize: '12px' }}>ANONYMIZED WAIT-FOR-GRAPH (WFG)</div>
           <div style={{ display: 'flex', gap: '12px', marginTop: '8px' }}>
              <div style={{ display: 'flex', alignItems: 'center', gap: '4px', fontSize: '10px' }}>
                 <div style={{ width: '8px', height: '8px', borderRadius: '50%', background: '#3b82f6' }}></div> Active Task
              </div>
              <div style={{ display: 'flex', alignItems: 'center', gap: '4px', fontSize: '10px' }}>
                 <div style={{ width: '8px', height: '8px', borderRadius: '50%', background: '#ff7e33' }}></div> Deadlocked (Cluster A)
              </div>
              <div style={{ display: 'flex', alignItems: 'center', gap: '4px', fontSize: '10px' }}>
                 <div style={{ width: '8px', height: '8px', borderRadius: '50%', background: '#ef4444' }}></div> Deadlocked (Cluster B)
              </div>
           </div>
        </div>
      </main>

      <aside className="sidebar-right">
        <h2 style={{ fontSize: '14px', color: '#71717a', marginBottom: '16px' }}>ENGINE STATUS</h2>
        
        <AnimatePresence>
          {data.clusters && data.clusters.length > 0 && (
            <motion.div 
              initial={{ opacity: 0, scale: 0.95 }}
              animate={{ opacity: 1, scale: 1 }}
              className="card" 
              style={{ borderLeft: '4px solid #ef4444', background: 'rgba(239, 68, 68, 0.05)' }}
            >
              <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#ef4444', fontWeight: '700', marginBottom: '12px' }}>
                <ShieldAlert size={18} /> CRITICAL: {data.clusters.length} DEADLOCKS
              </div>
              
              {data.clusters.map((cluster, idx) => (
                <div key={idx} style={{ marginBottom: '12px', padding: '8px', background: 'rgba(0,0,0,0.2)', borderRadius: '4px' }}>
                  <div style={{ fontSize: '11px', color: '#ef4444', fontWeight: 'bold' }}>CLUSTER #{idx + 1} ({cluster.length} TASKS)</div>
                  <div style={{ fontSize: '10px', color: '#71717a', marginTop: '4px', wordBreak: 'break-all' }}>
                    {cluster.slice(0, 3).join(', ')}{cluster.length > 3 ? '...' : ''}
                  </div>
                </div>
              ))}
            </motion.div>
          )}
        </AnimatePresence>

        <div className="card">
          <div style={{ display: 'flex', alignItems: 'center', gap: '8px', color: '#a1a1aa', fontSize: '12px', marginBottom: '8px' }}>
             <Activity size={14} /> OTEL SPAN OVERVIEW
          </div>
          <div className="otel-log">
            <div className="otel-entry"><span>[SYNC]</span> wfg_update</div>
            <div className="otel-entry"><span>[SPAN]</span> deadline_prune</div>
            <div className="otel-entry"><span>[METR]</span> sampling_recalc</div>
            <div className="otel-entry" style={{ color: '#ef4444' }}><span>[WARN]</span> scc_found_cycle</div>
          </div>
        </div>
      </aside>
    </div>
  );
};

export default App;
