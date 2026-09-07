#!/bin/bash
# =============================================================================
# run_all_agents.sh — orquestrador dos agentes de assets do fnxCraft
#
# Executa em sequência:
#   1. PBR Texture Agent   (gera _n/_s/_b dos blocos que faltam)
#   2. Item Texture Agent  (gera PNGs 16x16 de itens que faltam)
#   3. Asset Validator     (cross-check C++ <-> assets <-> shaders)
#   4. Build (opcional, --build)
#
# Uso:
#   ./scripts/agents/run_all_agents.sh            # só assets
#   ./scripts/agents/run_all_agents.sh --build    # assets + build
#   ./scripts/agents/run_all_agents.sh --verify   # só relatórios (nada é gerado)
# =============================================================================
set -u
DIR="$(cd "$(dirname "$0")/../.." && pwd)"
cd "$DIR"

VERIFY_FLAG=""
BUILD_FLAG=0
for arg in "$@"; do
	case "$arg" in
		--verify) VERIFY_FLAG="--verify" ;;
		--build)  BUILD_FLAG=1 ;;
	esac
done

C_GREEN='\033[0;32m'; C_RED='\033[0;31m'; C_BLUE='\033[0;34m'; C_NC='\033[0m'
FAIL=0

banner() { echo -e "${C_BLUE}============================================================${C_NC}"; echo -e "${C_BLUE} $1${C_NC}"; echo -e "${C_BLUE}============================================================${C_NC}"; }

banner "AGENT 1/3 — PBR Texture Agent (blocos)"
if python3 scripts/agents/pbr_texture_agent.py $VERIFY_FLAG; then
	echo -e "${C_GREEN}[OK] PBR agent${C_NC}"; else echo -e "${C_RED}[FAIL] PBR agent${C_NC}"; FAIL=1; fi

banner "AGENT 2/3 — Item Texture Agent (itens)"
if python3 scripts/agents/item_texture_agent.py $VERIFY_FLAG; then
	echo -e "${C_GREEN}[OK] Item agent${C_NC}"; else echo -e "${C_RED}[FAIL] Item agent${C_NC}"; FAIL=1; fi

banner "AGENT 3/3 — Asset Validator"
if python3 scripts/agents/validate_assets_agent.py; then
	echo -e "${C_GREEN}[OK] Validator${C_NC}"; else echo -e "${C_RED}[FAIL] Validator${C_NC}"; FAIL=1; fi

if [ "$BUILD_FLAG" = "1" ]; then
	banner "BUILD — client + server"
	if cmake --build build -j"$(nproc)"; then
		echo -e "${C_GREEN}[OK] build${C_NC}"; else echo -e "${C_RED}[FAIL] build${C_NC}"; FAIL=1; fi
fi

echo
if [ "$FAIL" = "0" ]; then
	echo -e "${C_GREEN}>>> TODOS OS AGENTES CONCLUIDOS COM SUCESSO <<<${C_NC}"
else
	echo -e "${C_RED}>>> ALGUNS AGENTES FALHARAM (veja acima) <<<${C_NC}"
fi
exit $FAIL
