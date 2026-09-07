const game = {
  setTheme(t){ document.getElementById('theme').href=`themes/${t}.css`; localStorage.setItem('theme',t); if(window.htmlUi) htmlUi.setTheme(t); },
  createWorld(name,diff,mode,keep,cheats){ if(!name) return alert('Nome vazio'); if(window.htmlUi) htmlUi.createWorld(name,diff,mode,keep,cheats); },
  playSelected(){ if(window.htmlUi) htmlUi.playSelected(); },
  openWorldConfig(){ if(window.htmlUi) htmlUi.openWorldConfig(); },
  openCargos(){ if(window.htmlUi) htmlUi.openCargos(); },
  openSettings(){ if(window.htmlUi) htmlUi.openSettings(); },
  refreshWorlds(list){ const el=document.getElementById('worldList'); el.innerHTML=''; list.forEach(w=>{ const d=document.createElement('div'); d.className='card'; d.innerHTML=`<b>${w.name}</b><br><small>${w.diff} • ${w.mode}</small>`; d.onclick=()=>htmlUi.selectWorld(w.name); el.appendChild(d); }); }
};
window.addEventListener('DOMContentLoaded',()=>{
  const t=localStorage.getItem('theme')||'dark';
  document.getElementById('theme').href=`themes/${t}.css`;
  if(window.htmlUi) htmlUi.requestWorlds();
});
