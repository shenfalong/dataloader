import numpy as np
import _pydb as db
import time
import cv2
import ctypes
import mxnet as mx
import pdb

param = { 'rec_path': '/mnt/WXRC0020/users/shenfalong/shen/datasets/ReID/train/convert2mxnet/train_xuhui.rec',
          'idx_path': '/mnt/WXRC0020/users/shenfalong/shen/datasets/ReID/train/convert2mxnet/train_xuhui.txt',
        'batch_size': 64}
db.reset(param)

data = mx.nd.empty((64,3,224,224), dtype='uint8', ctx=mx.gpu(0))
mx.base._LIB.MXNDArrayWaitToWrite(data.handle)
data_ptr = ctypes.c_void_p()
mx.base._LIB.MXNDArrayGetData(data.handle, ctypes.byref(data_ptr))

label = mx.nd.empty((64,), dtype='float32', ctx=mx.gpu(0))
mx.base._LIB.MXNDArrayWaitToWrite(label.handle)
label_ptr = ctypes.c_void_p()
mx.base._LIB.MXNDArrayGetData(label.handle, ctypes.byref(label_ptr))

t = time.time()
prev_t = t
for i in range(0,100000000):
  db.next(data_ptr, label_ptr)
  #batch = np.reshape(batch, (64,224,224,3));
  #cv2.imwrite('1.png',batch[0,:,:,:])
  #pdb.set_trace()
  #print(len(batch['im']))
  if i % 100 == 0:
  	print(t - prev_t)
  	prev_t = t
  	t = time.time()
  #pdb.set_trace()
  #cv2.imwrite('%d.png'%i,batch['im'][i,:,:,:])
  
