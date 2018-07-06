# Script for extracting feature or data from Caffe

"""
  This script is used to extract feature map or parameters
  from Caffe toolkit. 

"""

import numpy as np
import matplotlib.pyplot as plt
import sys

# Add Caffe path
caffe_home = '/home/yueniu/caffe/'
sys.path.insert(0, caffe_home + 'python')
import caffe

import os


# Load model and weights
if os.path.isfile(caffe_home + 'models/bvlc_vggnet/VGG_ILSVRC_16_layers_fc6_256_fc7_256.caffemodel'):
  print '[INFO] CaffeNet found.'
else:
  print '[ERROR] No CaffeNet file.'

caffe.set_mode_cpu()
model_def = caffe_home + 'models/bvlc_vggnet/deploy_fc6_256_fc7_256.prototxt'
model_param = caffe_home + 'models/bvlc_vggnet/VGG_ILSVRC_16_layers_fc6_256_fc7_256.caffemodel'
net = caffe.Net(model_def, model_param, caffe.TEST)

# Extract param
fp32_handle = open(r"../data/param.bin", "wb")
fp16_handle = open(r"../data/paramfp16.bin", "wb")
# Conv layer
conv_lyr = ('conv1_1', 'conv1_2', 
            'conv2_1', 'conv2_2', 
            'conv3_1', 'conv3_2', 'conv3_3', 
            'conv4_1', 'conv4_2', 'conv4_3', 
            'conv5_1', 'conv5_2', 'conv5_3')
for lyr in conv_lyr:
  print '[INFO] Extract params in layer', lyr
  weights = net.params[lyr][0].data
  bias = net.params[lyr][1].data
  dim = weights.shape
  
  # Convert to fp16
  weights_fp16 = np.array(weights, dtype = np.float16)
  bias_fp16 = np.array(bias, dtype = np.float16)

  if 'conv1_1' == lyr:
    itile = 3
  else:
    itile = 16
  otile = 32
  
  # Write bias first
  fp32_handle.write(bias)
  fp16_handle.write(bias_fp16)
  # Write weights
  for isec in range(0, dim[1] / itile):
    for osec in range(0, dim[0] / otile):
      fp32_sec = weights[osec*otile:(osec+1)*otile, isec*itile:(isec+1)*itile,:,:].copy()
      fp32_handle.write(fp32_sec)
      fp16_sec = weights_fp16[osec*otile:(osec+1)*otile, isec*itile:(isec+1)*itile,:,:].copy()
      fp16_handle.write(fp16_sec)

# FC layer
fc_lyr = ('fc6_1','fc6_2','fc7_1','fc7_2','fc8')
for lyr in fc_lyr:
  print '[INFO] Extract param in layer', lyr
  weights = net.params[lyr][0].data
  dim = weights.shape
  if 'fc6_2' == lyr or 'fc7_2' == lyr or 'fc8' == lyr:
    bias = net.params[lyr][1].data
  else:
    bias = np.zeros((dim[0]), dtype = np.float32)

  # Convert to fp16
  weights_fp16 = np.array(weights, dtype = np.float16)
  bias_fp16 = np.array(bias, dtype = np.float16)

  # Write bias fist
  fp32_handle.write(bias)
  fp16_handle.write(bias_fp16)

  # Change weights mem arrangement
  weights_rearr = weights_fp16.copy()
  if 'fc6_1' == lyr:
    for ochnl in range(0, dim[0]):
      for row in range(0, 7):
        for ichnl in range(0, 512):
          offset1 = row * 512 * 7 + ichnl * 7;
          offset2 = ichnl * 7 * 7 + row * 7;
          weights_rearr[ochnl, offset1:offset1+7] = weights_fp16[ochnl, offset2:offset2+7].copy()

  if(dim[0] < 1024):
    for isec in range(0, dim[1]/64):
      fp16_handle.write(weights_rearr[:, isec * 64 : (isec+1) * 64].copy());  
  else:
    for osec in range(0, dim[0]/1024):
      for isec in range(0, dim[1]/64):
        fp16_handle.write(weights_rearr[osec * 1024:(osec+1) * 1024, isec * 64 : (isec+1) * 64].copy())
       
  # Write weights
  fp32_handle.write(weights)

fp32_handle.close()
fp16_handle.close()
