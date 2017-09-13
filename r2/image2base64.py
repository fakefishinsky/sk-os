#encoding:utf-8
import base64
f = open('F:/audit-logo.jpg','rb')
res = base64.b64encode(f.read())
f.close()

ff = open('F:/audit-logo.txt','w')
ff.write(res)
ff.close()