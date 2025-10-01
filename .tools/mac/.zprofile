

# Python 3.12
PATH="/Library/Frameworks/Python.framework/Versions/3.12/bin:${PATH}"
export PATH

# === ESP-IDF CONFIG ===
export IDF_PATH="/Users/florinbaciu/esp/v5.4.2/esp-idf"
source "$IDF_PATH/export.sh"
#export PATH="/Users/florinbaciu/esp/v5.4.2/esp-idf/components/espcoredump:$PATH"

# MacPorts Installer
export PATH="/opt/local/bin:/opt/local/sbin:$PATH"

# === ALIASES ===
alias py="python3"
alias python="python3"
alias pip="pip3"
alias idf="idf.py"
