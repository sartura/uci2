cd ../src
flex uci2_cfg.l
bison -d uci2_cfg.y
cd - > /dev/null
echo "Lexer and Parser successfully generated."
