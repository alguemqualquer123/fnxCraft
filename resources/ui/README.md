# UI fnxCraft - HTML/CSS/JS por componente

Cada UI é uma pasta separada em `resources/ui/<nome>/` com 3 arquivos:
- `index.html` - estrutura
- `style.css` - tema local (usa vars de `themes/dark.css`)
- `script.js` - lógica via `window.htmlUi` (bindings C++)

Componentes reutilizáveis em `components/` (button.html, card.html) via `<template>` + JS `fetch`.

Para criar nova UI (ex `loja`):
```bash
mkdir resources/ui/loja
echo '<div>Loja</div>' > resources/ui/loja/index.html
echo '.loja{...}' > resources/ui/loja/style.css
echo 'htmlUi.onLoja = (data)=>{...}' > resources/ui/loja/script.js
# C++: gHtmlUi.loadComponent("loja") ou htmlUi.open('loja')
```

Hot-reload em dev: `HtmlUiEngine` observa `resources/ui/**` com `FileWatcher` e recarrega View sem recompilar.
Escalável: novas UIs não afetam existentes; temas trocados via `themes/*.css` e `localStorage`.
