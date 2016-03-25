
--local print = log_message


local sfind = string.find
local smatch = string.match
local slen = string.len
local srep = string.rep
local sgsub = string.gsub
local tinsert = table.insert
local sbyte = string.byte
local slower = string.lower
local supper = string.upper
local fopen = io.open
local tpack = pack.tpack
local pack = pack.pack
local newguid = os.guid

local P, R, S, C, V, Ct, Cf, Cg =
  lpeg.P, lpeg.R, lpeg.S, lpeg.C, lpeg.V, lpeg.Ct, lpeg.Cf, lpeg.Cg



local DEBUG=false

local function parse(xmlstr)
	local stack={}
	local top={}
	table.insert(stack,top)

	local function argumenttable(...)
		local arguments={...}
		local args
		
		if #arguments>0 then 
			args={}
			for i=1,#arguments,2 do
				args[arguments[i]]=arguments[i+1]
			end
		end
		return args
	end
	
	local function open(label,...)
		if DEBUG then print("open",label,...) io.flush() end
		top = {label=label,args=argumenttable(...)}
		table.insert(stack, top)
	end

	local function close(label)
		if DEBUG then print("close",label,stack[#stack].label) io.flush() end
		local toclose = table.remove(stack)  -- remove top
		top = stack[#stack]
		table.insert(top, toclose)
	end

	local function single(label,...)
		if DEBUG then print("single",label) io.flush() end
		open(label,...)
		close(label)
	end

	local function text(txt)
		if DEBUG then print("text",'#'..txt..'#') end
		table.insert(top,txt)
	end

	local function cdata(txt)
		if DEBUG then print("cdata",'#'..txt..'#') end
		table.insert(top,txt)
	end

	local function comment(txt)
		if DEBUG then print("comment",'#'..txt..'#') end
		if not top.comment then top.comment={} end
		top.comment[#top.comment+1]=txt
	end

	local function declaration(name,...)
		if DEBUG then print("declaration",...) io.flush() end
		if not top.declaration then top.declaration={} end
		top.declaration[name]=argumenttable(...)
	end

	local lt=lpeg.P('<')
	local gt=lpeg.P('>')
	local apos=lpeg.P('"')
	local equal=lpeg.P("=")
	local slash=lpeg.P("/")
	local space=lpeg.S(" \t\r\n")
	local letter=lpeg.R("az", "AZ")
	local namecharstart=letter + '_' + ':'
	local name=namecharstart * (namecharstart + lpeg.R("09") + lpeg.P('-'))^0
	local content=(lpeg.P(1) - lt)^1
	local comment_open=lpeg.P("<!--")
	local comment_close=lpeg.P("-->")
	local cdata_start=lpeg.P("<![CDATA[")
	local cdata_end=lpeg.P("]]>")

	local declaration=(space^0 * lpeg.P('<?') * lpeg.C(name) * 
		(space^1 * lpeg.C(name) * equal * 
		apos * lpeg.C((lpeg.P(1) -apos)^1) * apos )^1 * 
		space^0 * lpeg.P('?>')) / declaration
		
	local attribute=(space^1 * lpeg.C(name) * space^0 * equal * space^0 * 
		apos * lpeg.C((lpeg.P(1) - apos)^0) * apos)
		
	local opening_element=(space^0 * lt * lpeg.C(name) * 
		attribute^0 * space^0 * gt) / open
		
	local closing_element=(space^0 * lt * slash * lpeg.C(name) * gt) / close
	
	local singelton_element=(space^0 * lt * lpeg.C(name) * 
		attribute^0 * space^0 * slash *gt) / single
		
	local content=space^0 * ((lpeg.P(1) - lt)^1) * space^0 / text
	
	local cdata=(space^0 * cdata_start  * 
		lpeg.C((lpeg.P(1) -cdata_end)^1) * cdata_end )/cdata
		
	local comment_element=(space^0 * comment_open * 
		lpeg.C((lpeg.P(1) - comment_close)^0) * comment_close) / comment

	local xml=lpeg.P{
		[1]=declaration^0 * lpeg.V(2),
		[2]=opening_element * lpeg.V(3)^0 * closing_element,
		[3]=comment_element^1 + singelton_element^1 
			+ content + cdata + lpeg.V(2),
	}

	if xml:match(xmlstr) then
		return stack[1]
	else
		return nil,"Parse error"
	end
end
	
local function tprint (t, indent, done)
  -- show strings differently to distinguish them from numbers
  local function show (val)
    if type (val) == "string" then
      return '"' .. val .. '"'
    else
      return tostring (val)
    end -- if
  end -- show
  -- entry point here
  if not t then
	print"nil"
	return
  end
  done = done or {}
  indent = indent or 0
  for key, value in pairs (t) do
    io.write (string.rep (" ", indent)) -- indent it
    if type (value) == "table" and not done [value] then
      done [value] = true
      print (show (key) ..  ":");
      tprint (value, indent + 2, done)
    else
      io.write (show (key) ..  "=")
      print (show (value))
    end
  end
end

local function trim (s)
      return (sgsub(s, "^%s*(.-)%s*$", "%1"))
end



local function split (s, sep)
  sep = P(sep) ^ 1
  local elem = C((1 - sep)^1)
  local p = Ct(elem * (sep * elem)^0)   -- make a table capture
  return p:match(s)
end

local function split_once(s, sep)
	sep = P(sep) ^ 1
	local elem = C((1 - sep)^1)
	local other = C(P(1)^0)
	local p = Ct(elem * sep * other)
	return p:match(s)
end

local function split_path(path)
	local folder, file = smatch(path,"^(.-)([^:/\\]*)$")
	return folder, file
end

local function item_in_list(v, list)
	for i, x in ipairs(list) do
		if v == x then
			return i
		end
	end
	return nil

end





local function nodetree(tbl, objstree, parent)
	
	local objname = ""

	local objs = objstree.objects

	if tbl.label ~= nil and slen(tbl.label) > 0 and tbl.args then
		objname = tbl.label
		objs[objname] = {}
		objs[objname].args = tbl.args

		local f = tbl.args.Callback
		if f and slen(f) > 0 and nil == item_in_list(f, objstree.callbacks) then
			tinsert(objstree.callbacks, f)
		end

		f = tbl.args.Keymap
		if f and slen(f) > 0 and nil == item_in_list(f, objstree.keymaps) then
			tinsert(objstree.keymaps, f)
		end

		-- add this object to parent
		if parent then
			if nil == objs[parent].children then
				objs[parent].children = {}
			end
			objs[objname].parent = parent
			tinsert(objs[parent].children, objname)
		else
			objs.root = objname
		end

	end
	
	for k,v in ipairs(tbl) do
		if type(v) == "table" then
			nodetree(v, objstree, objname)
		end
	end

end

local object_type_names = {
	TEXT_FIELD		= "OT_TEXTFIELD",
	EDIT_FIELD		= "OT_EDITFIELD",
	BITMAP		= "OT_BITMAP",
	MULTISEL		= "OT_MULTISEL",
	PROGRESS_BAR	= "OT_PROGRESSBAR",
	SCROLL_BAR		= "OT_SCROLLBAR",
	MULTI_TEXT		= "OT_MULTITEXT",
	OBJLIST			= "OT_OBJLIST",
	CONTAINER		= "OT_CONTAINER",
}

local function valid_val(v, default)
	if not v then
		return default
	end
	if(slen(v) == 0) then
		return default
	elseif (v == '(null)') then
		return 'NULL'
	end
	return v
end


local function next_obj(list, obj)
	if not list then
		return "NULL"
	end

	for i, v in ipairs(list) do
		if v == obj then
			if i < #list then
				return list[i + 1]
			end
		end
	end
	return "NULL"
end

local function gen_functions(args, fw)
	if (slen(args.Keymap) > 0) then
		fw:write("static VACTION ", args.Keymap, "(POBJECT_HEAD pObj, UINT32 key);\n")
	end
	if (slen(args.Callback) > 0) then
		fw:write("static PRESULT ", args.Callback, "(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);\n")
	end
	fw:write("\n")

end


local function objecthead_h(objtree, name, fw)
	if not objtree[name].args then
		return
	end

	local obj = objtree[name].args
	fw:write("\t.head = {\n")
	fw:write("\t\t", ".bType = ", object_type_names[obj.Type], ",\n")
	fw:write("\t\t", ".bAttr = ", obj.Attribute, ", .bFont = ", obj.Font, ",\n")
	fw:write("\t\t", ".bID = ", obj.ID, ", .bLeftID = ", valid_val(obj.ID_Left, '0'), ", .bRightID = ", 
						valid_val(obj.ID_Right, '0'), ", .bUpID = ", valid_val(obj.ID_Up, '0'), ", .bDownID = ", valid_val(obj.ID_Down, '0'), ",\n" )
	fw:write("\t\t", ".frame = {", obj.Left, ", ", obj.Top, ", ", obj.Width, ", ", obj.Height, "},\n")
	fw:write("\t\t", ".style = {\n")
	local showstyle = valid_val(obj.Style_Show, "0")
	fw:write("\t\t\t", ".bShowIdx = ",showstyle , ", .bHLIdx = ", valid_val(obj.Style_Highlight, showstyle),
				", .bSelIdx = ", valid_val(obj.Style_Select, showstyle), ", .bGrayIdx = ", valid_val(obj.Style_Gray, showstyle), "\n")
	fw:write("\t\t", "},\n")
	fw:write("\t\t", ".pfnKeyMap = ",  valid_val(obj.Keymap, 'NULL'), ", .pfnCallback = ",  valid_val(obj.Callback, 'NULL'), ",\n")
	
	local o =  objtree[name]
	local parent = o.parent
	
	local nxt = next_obj(objtree[parent].children, name)

	if parent == objtree.root then
		parent = "NULL"
	end
	
	if nxt ~= 'NULL' then
		nxt = "&" .. nxt
	end

	if parent ~= 'NULL' then
		parent = "&" .. parent
	end
		

	fw:write("\t\t", ".pNext = (POBJECT_HEAD)",nxt,", .pRoot = (POBJECT_HEAD)",parent, "\n")
	fw:write("\t},\n")


end

local function bitmap_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)

	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.bAlign = ", valid_val(obj.Align, 'C_ALIGN_CENTER'), ",\n")
	fw:write("\t.bX = ", valid_val(obj.bX, '0'), ", .bY = ", valid_val(obj.bY, '0'), ",\n")
	fw:write("\t.wIconID = ", valid_val(obj.BitmapID, '0'), ",\n")
	fw:write("};\n")

end

local function container_h(objtree, name, fw)
	local obj = objtree[name].args
	
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)

	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)

	firstchild = "NULL"
	local children = objtree[name].children
	if children and #children > 0 then
		firstchild = "&" .. children[1]
	end
	fw:write("\t.pNextInCntn = (POBJECT_HEAD)", firstchild, ",\n")
	
	local fid = nil
	if obj.Focus_Object then
		local fobj = objtree[obj.Focus_Object]
		if fobj then
			fid = fobj.args.ID
		end
	end
	fw:write("\t.FocusObjectID = ", valid_val( fid, "0"), ",\n")
	fw:write("\t.bHiliteAsWhole = ", valid_val( obj.HiliteAsWhole, "0"), ",\n")
	fw:write("};\n")

end

local function editfield_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)

	pat = "NULL"
	if slen(obj.Pattern) > 0 then
		fw:write("static char ", name, '_pattern[] = "', obj.Pattern, '";\n')
		pat = name .. '_pattern'
	end

	prefix = "NULL"
	if slen(obj.Prefix) > 0 then
		prefix = name .. '_Prefix'
		fw:write('static const UINT8 ', prefix, '[] = {' )

		local slen = slen(obj.Prefix)
		for i = 1, slen do
			fw:write("0, ", sbyte(obj.Prefix, i), ", ")
		end
		fw:write('0, 0}; //Prefix = ', obj.Prefix, "\n")
	end
	
	suffix = "NULL"
	if slen(obj.Suffix) > 0 then
		suffix = name .. '_Suffix'
		fw:write('static const UINT8 ', suffix, '[] = {' )
		local slen = slen(obj.Suffix)
		for i = 1, slen do
			fw:write("0, ", sbyte(obj.Suffix, i), ", ")
		end
		fw:write('0, 0}; //Suffix = ', obj.Suffix, "\n")

	end
	
	pstring = 'NULL'
	if slen(obj.String) > 0 then
		pstring = obj.String
	end

	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.bAlign = ", valid_val(obj.Align, 'C_ALIGN_CENTER'), ",\n")
	fw:write("\t.bX = ", valid_val(obj.bX, '0'), ", .bY = ", valid_val(obj.bY, '0'), ",\n")
	fw:write("\t.bMaxLen = ", valid_val(obj.MaxLength, '16'), ",\n")
	fw:write("\t.pcsPattern = ", pat, ",\n")
	fw:write("\t.bStyle = ", valid_val(obj.Style, 'CURSOR_NORMAL'), ",\n")
	fw:write("\t.bCursorMode = ", valid_val(obj.CursorMode, "CURSOR_NORMAL"), ', .bCursor = ', valid_val(obj.Cursor, '0'), ",\n")
	fw:write("\t.pString = ", pstring, ",\n")
	fw:write("\t.pPrefix = (UINT16*)", prefix, ", .pSuffix = (UINT16*)", suffix, ",\n")
	fw:write("\t.valbak = 0\n")
	fw:write("};\n")

end

local function multisel_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)
	

	local seltable = 'NULL'
	if obj.SelType == 'STRING_ID' then
		if slen(obj.SelTable) > 0 then
			seltable = name .. "_seltable"
	
			fw:write("static UINT16 ", name, "_seltable[] = {\n")
			local sts = split(obj.SelTable, "|")
			for _, v in ipairs(sts) do
				fw:write("\t", v, ",\n")
			end
			fw:write("};\n")
		end
	elseif obj.SelType == 'STRING_NUMBER'  or obj.SelType == 'STRING_NUM_TOTAL' then
		if slen(obj.SelTable) > 0 then
			seltable = name .. "_seltable"
	
			fw:write("static UINT32 ", name, "_seltable[] = {")
			local sts = split(obj.SelTable, ",")
			for _, v in ipairs(sts) do
				fw:write(v, ", ")
			end
			fw:write("};\n")
		end
	else
		if slen(obj.SelTable) > 0 then
			seltable = obj.SelTable
		end
	end

	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.bAlign = ", valid_val(obj.Align, 'C_ALIGN_CENTER'), ",\n")
	fw:write("\t.bX = ", valid_val(obj.bX, '0'), ", .bY = ", valid_val(obj.bY, '0'), ",\n")
	fw:write("\t.bSelType = ", valid_val(obj.SelType, 'STRING_ID'), ",\n")
	fw:write("\t.pSelTable = ", seltable, ",\n")
	fw:write("\t.nCount = ", valid_val(obj.nCount, "0"), ",\n")
	fw:write("\t.nSel = 0,\n")
	fw:write("};\n")

end

local function scrollbar_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)

	
	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.bStyle = ", valid_val(obj.Style, 'BAR_VERT_AUTO|SBAR_STYLE_RECT_STYLE'), ",\n")
	fw:write("\t.bPage = ", valid_val(obj.Page, '100'),  ",\n")
	fw:write("\t.wThumbID = ", valid_val(obj.StyleThumb, 'NULL'), ", .wTickBg = ", valid_val(obj.StyleTickBackground, 'NULL'), ",\n")
	fw:write("\t.rcBar = {", obj.BarLeft, ", ", obj.BarTop, ", ", 
		obj.BarWidth, ", ", obj.BarHeight, "},\n")

	fw:write("\t.nMax = ", valid_val(obj.Max, "100"), ", .nPos = ", valid_val(obj.Pos, '0'), "\n")
	fw:write("};\n")

end

local function multitext_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)

	
	tcnt = name .. "_text_content"
	fw:write("TEXT_CONTENT ", tcnt, "[] = {\n")
	fw:write("\t{", valid_val(obj.TextType), ", ")
	if obj.TextType == 'STRING_ID' then
		fw:write(valid_val(obj.StringID, "0" ))
	else
		fw:write(valid_val(obj.String, "0"))
	end
	fw:write("},\n};\n")


	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.bAlign = ", valid_val(obj.Align, 'C_ALIGN_CENTER'), ",\n")
	fw:write("\t.bCount = ", valid_val(obj.Count, '1'),  ",\n")
	fw:write("\t.nLine = ", valid_val(obj.Line, '0'),  ",\n")
	fw:write("\t.rcText = {", obj.TextLeft, ", ", obj.TextTop, ", ", 
		tonumber(obj.TextWidth), ", ", tonumber(obj.TextHeight), "},\n")

	scrollbar = valid_val(obj.ScrollBar, 'NULL')
	if scrollbar ~= 'NULL' then
		scrollbar = '&' .. scrollbar
	end
	fw:write("\t.scrollBar = ", scrollbar, ",\n")
	fw:write("\t.pTextTable = ", tcnt, "\n")
	fw:write("};\n")

end

local function textfield_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)



	pstring = 'NULL'
	if slen(obj.String) > 0 then
		pstring = obj.String
	end

	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.bAlign = ", valid_val(obj.Align, 'C_ALIGN_CENTER'), ",\n")
	fw:write("\t.bX = ", valid_val(obj.bX, '0'), ", .bY = ", valid_val(obj.bY, '0'), ",\n")
	fw:write("\t.wStringID = ", valid_val(obj.StringID, "0"), ",\n")
	fw:write("\t.pString = ", pstring, "\n")
	fw:write("};\n")

end

local function objlist_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	
	gen_functions(obj, fw)


	children = objtree[name].children
	chs = "NULL"
	if children and #children > 0 then
		chs = name .. "_ListField"
		fw:write("static POBJECT_HEAD ", chs, "[] = {\n")
		for _, v in ipairs(children) do
			if objtree[v] and objtree[v].args and objtree[v].args.Type ~= "SCROLL_BAR" and v ~= obj.SelMarkObject then
				fw:write("\t(POBJECT_HEAD)&", v, ",\n")
			end
		end
		fw:write("};\n")

	end


	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.pListField = ",chs, ",\n")
	
	scrollbar = valid_val(obj.ScrollBar, 'NULL')
	if scrollbar ~= 'NULL' then
		scrollbar = '&' .. scrollbar
	end
	fw:write("\t.scrollBar = ",scrollbar, ",\n")
	fw:write("\t.bListStyle = ",valid_val(obj.Style, '0'), ",\n")
	fw:write("\t.wDep = ", valid_val(obj.Dep, '0'), ", .wCount = ", valid_val(obj.Count, '0'), ",\n")
	fw:write("\t.wTop = 0, .wCurPoint = 0, .wNewPoint = 0,\n")
	fw:write("\t.wSelect = 0, .dwSelect = NULL,\n")

	selmask = valid_val(obj.SelMarkObject, 'NULL')
	if selmask ~= 'NULL' then
		selmask = '(POBJECT_HEAD)&' .. selmask
	end
	fw:write("\t.pSelMarkObj = ", selmask, "\n")
	fw:write("};\n")

end

local function progressbar_h(objtree, name, fw)
	local obj = objtree[name].args
	fw:write("\n//", name, " ------------------------------------------------------------------------\n")
	gen_functions(obj, fw)

	
	fw:write(obj.Type, " ", name, " = {\n")
	objecthead_h(objtree, name, fw)
	fw:write("\t.bStyle = ", valid_val(obj.Style, 'PROGRESSBAR_HORI_NORMAL'), ",\n")
	fw:write("\t.bX = ", valid_val(obj.bX, '0'), ", .bY = ", valid_val(obj.bY, '0'), ",\n")
	fw:write("\t.wTickBg = ", valid_val(obj.StyleTickBackground, 'NULL'), ", .wTickFg = ", valid_val(obj.StyleTickForeground, 'NULL'), ",\n")
	fw:write("\t.rcBar = {", obj.BarLeft, ", ", obj.BarTop, ", ", 
		obj.BarWidth, ", ", obj.BarHeight, "},\n")
	fw:write("\t.nMin = ", valid_val(obj.Min, '1'), ", .nMax = ", valid_val(obj.Max, '100'), ",\n")
	fw:write("\t.nBlocks = ", valid_val(obj.Blocks, '100'), ", .nPos = ", valid_val(obj.Pos, '0'), ",\n")
	fw:write("};\n")

end

local  __to_h = {
	TEXT_FIELD	= textfield_h,
	EDIT_FIELD	= editfield_h,
	BITMAP	= bitmap_h,
	MULTISEL	= multisel_h,
	PROGRESS_BAR	= progressbar_h,
	SCROLL_BAR	= scrollbar_h,
	MULTI_TEXT	= multitext_h,
	OBJLIST		= objlist_h,
	CONTAINER	= container_h,
}

local function __to_common_callback(f, fw)

	fw:write("static PRESULT ", f, "(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2)\n")
	fw:write([[{
	PRESULT ret = PROC_PASS;
	VACTION unact;

	switch (event)
	{
		case EVN_PRE_OPEN:
			//TODO: add code here
			break;
		case EVN_POST_OPEN:
			//TODO: add code here
			break;
		case EVN_PRE_CLOSE:
			//TODO: add code here
			break;
		case EVN_POST_CLOSE:
			//TODO: add code here
			break;
		case EVN_UNKNOWNKEY_GOT:
			//TODO: add code here
			break;
		case EVN_UNKNOWN_ACTION:
			//TODO: add code here
			break;
		case EVN_MSG_GOT:
			//TODO: add code here
			break;
	}
	return ret;
}
]])
	
end

local function __to_common_keymap(f, fw)

	fw:write("static VACTION ", f, "(POBJECT_HEAD pObj, UINT32 key)\n")
	fw:write([[{
	VACTION act = VACT_PASS;
	UINT32 hKey;

	switch (key)
	{
		case V_KEY_EXIT:
		case V_KEY_MENU:
		case V_KEY_SUBTITLE:
			act = VACT_CLOSE;
			break;
		case V_KEY_GREEN:
			//TODO: add code here
			break;
		case V_KEY_POWER:
			power_switch(0);
			break;
		default:
			act = VACT_PASS;
}

	return act;

}
]])

end


local function _to_functions(objtree, fw)
	
	local maps = objtree.keymaps
	for _, v in ipairs(maps) do
		__to_common_keymap(v, fw)
	end

	local cbs = objtree.callbacks
	for _, v in ipairs(cbs) do
		__to_common_callback(v, fw)
	end

end

function xform_to_h_r(objtree, objname, fw)
	-- first output itself
	args = objtree[objname].args
	if args.Type then
		if args.Type ~= "Form" then
			__to_h[args.Type](objtree, objname, fw)
		end
	end
	children = objtree[objname].children

	if children and #children > 0 then
		for _, v in ipairs(children) do
			 xform_to_h_r(objtree, v, fw)
		end
	end

end
function xform_to_h(objtree, fw)
	xform_to_h_r(objtree, objtree.root, fw)
end

local function _file_header(form, fw)
	fw:write([[#include <sys_config.h>

#include <types.h>
#include <basic_types.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/hld_dev.h>
#include <hld/osd/osddrv_dev.h>
]])
	
	if form and form.args and form.args.OSDBits and tonumber(form.args.OSDBits) > 8 then
		fw:write("#include <api/libge/osd_lib.h>\n")
	else
		fw:write("#include <api/libosd/osd_lib.h>\n")
	end
		
	fw:write([[
#include "osdobjs_def.h"
#include "images.id"
#include "string.id"
#include "osd_config.h"

#include "control.h"
#include "menus_root.h"
#include "win_com.h"
#include "win_com_popup.h"
#include "win_com_list.h"
]])

end

local function _file_declare(predata, objects, fw)
	fw:write([[


/*******************************************************************************
*	Objects definition
*******************************************************************************/
]])
	-- output the pre defined content
	if predata and type(predata) == "string" then
		fw:write("//variables for objects defined here!\n")
		fw:write(predata, "\n\n")
	end

	-- firstly declare all the objects
	for k, v in pairs(objects) do
		if v.args and v.args.Type and v.args.Type ~= "Form"  then
			fw:write("extern ", v.args.Type, " ", k , ";\n" );
		end
	end
	-- export all the control objects
	xform_to_h(objects, fw)

end
local function _file_body(tree, fw)
	fw:write([[


/*******************************************************************************
*	Callback and Keymap Functions
*******************************************************************************/
]])
	_to_functions(tree, fw)

end


function xform_parser(path, filename, mode)
	local fh = fopen(path)
	if fh == nil then
		print("file: " .. path .. " doesn't exist!\n")	
		return 0
	end
	local input = fh:read'*a'
	fh:close()
	local form = parse(input)
	if nil == form then
		print(path .. " parse failed!\n")
		return 0
	end

	tree = {}
	tree.objects = {}
	tree.callbacks = {}
	tree.keymaps = {}


	nodetree(form[1], tree, nil)
	

	
	--output file
	local fw, err = io.open(filename, "w")
	if nil == fw then
		print("Open file: " .. filename .. " failed! error is " .. (err or "unknow error") .. "\n")
		return 0
	end
	---------------------------------------------------------
	-- mode 0: the whole file
	-- mode 1: only the header file
	mode = mode or 0

	
	if 1 == mode then
		local _, f = split_path(path)
		local hf = supper(sgsub(f, "%..*$", "_h"))
			
		fw:write([[/***************************************************************************************************
	This header file is generated by the Vega from a xform file.
	Please don't modify it!!
---------------------------------------------------------------------------------------------------*/
		]])
		hf = "__" .. hf .. "__" .. newguid()
		fw:write("\n#ifndef ", hf, "\n")
		fw:write("#define ", hf, "\n")
		_file_declare(form[1][1], tree.objects, fw)	
		fw:write("#endif//", hf, "\n")

	elseif 2 == mode then
		local _, f = split_path(path)
		local hf = sgsub(f, "%..*$", ".h")
		_file_header(form[1], fw)
		fw:write("\n//include the header from xform \n")
		fw:write('#include "', hf, '"\n')
		_file_body(tree, fw)

	elseif 3 == mode then
		_file_declare(form[1][1], tree.objects, fw)	

	else
		_file_header(form[1], fw)
		_file_declare(form[1][1], tree.objects, fw)	
		_file_body(tree, fw)
	end

	fw:close()

	print("xform: [" .. path .. "] -> [" .. filename .. "] successed\n")

	return 1
end



--- run this script from command line
if #arg < 2 then
	print("Usage: lua xform.lua xform_file c_file")
else
	xform_parser(arg[1], arg[2], 1)
end
