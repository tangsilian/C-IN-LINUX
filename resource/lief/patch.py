import lief

hashme = lief.parse("a.out")
print(hashme)
# 获取get puts函数的 Dynamic symbols
#python2中返回的是过滤后的列表, 而python3中返回到是一个filter类  加上.__next__() 是和原来等价的
puts_sym=filter(lambda e: e.name == "puts", hashme.imported_symbols).__next__()
print(puts_sym)
#将puts换为system
puts_sym.name = "system"
#改动写入新的文件
hashme.write("a.out.patch")
print("done")