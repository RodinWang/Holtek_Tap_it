#GenBeats.py

i=0
beat=0
x=67

f=open("D:\\python\\beats.txt",mode="w")

for i in range(0,264):
    # print("%d," %i,f)
    f.write("%d, " % beat)
    beat+=x

f.close()