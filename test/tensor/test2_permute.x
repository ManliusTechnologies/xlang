import CpuTensor as T
t3 =  t1*T.permute([2,1,0])*t2
t_g = T.graph(t3)
t_g.run()


t1 = tensor([[1,2,3],[4,5,6],[7,8,9]],name="t1")
t2 = tensor([[10,20,30],[40,50,60],[70,80,90]],name="t2")
t3 =tensor([0.1,0.2,0.3],name="t3")
t4 =  t2+t1*T.permute([1,0])*T.div()*t3
t5 = t4+t3*(t1*t3+t1*t2)
t_g = T.graph(t4,t5)
print(t_g)
t_g.run()
