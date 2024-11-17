# LuaLexer
适用于Lua5.3.x的Lua词法解析器

### Lua 示例代码

```lua
local lexer = require("lualexer")

-- Lua代码
local input = "local x = 42"

-- 进行词法解析
local tokens = lexer.tokenize(input)

-- 输出类型和值
for i, token in ipairs(tokens) do
    print(string.format("Token %d: Type = %s, Value = %s", i, token.type, token.value))
end
