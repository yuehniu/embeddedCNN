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


# Set input
mu = np.load(caffe_home + 'python/caffe/imagenet/ilsvrc_2012_mean.npy')
mu = mu.mean(1).mean(1)
print '[INFO] Mean subtracted values:', zip('BGR', mu)

transformer = caffe.io.Transformer({'data': net.blobs['data'].data.shape})
transformer.set_transpose('data', (2, 0, 1))
transformer.set_mean('data', mu)
transformer.set_raw_scale('data', 255)
transformer.set_channel_swap('data', (2, 1, 0))

# CPU forward
net.blobs['data'].reshape(1, 3, 224, 224)
image = caffe.io.load_image(caffe_home + 'examples/images/cat.jpg')
transformer_image = transformer.preprocess('data', image)
plt.imshow(image)
net.blobs['data'].data[...] = transformer_image;
output = net.forward()
output_prob = output['prob'][0]
print '[INFO] Predicted class is:', output_prob.argmax()

# Extract data
img = net.blobs['fc8'].data
print '[INFO] Data shape:', img.shape

# Convert FP32 to FP16
_fp16_ = True
if _fp16_:
  print '[INFO] Convert FP32 to FP16...'
  img_fp16 = np.array(img, dtype = np.float16)

_reshape_ = False
if _reshape_:
  print '[INFO] Rearrange data storage...'
  til_num = 14;
  chnl = 512;
  img_re = np.zeros((til_num, chnl, til_num), dtype=np.float16);
  for ch in range(0, chnl):
    for row in range(0, til_num):
      img_re[row, ch, :] = img_fp16[0, ch, row, :];

print '[INFO] Write data to disk...'
file_handle = open(r"../data/fc8.bin", "wb")
file_handle.write(img)
file_handle.close()
file_handle = open(r"../data/fc8fp16.bin", "wb")
file_handle.write(img_fp16)
file_handle.close()

# Show image
plt.show()
