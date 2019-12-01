#!/bin/bash

source activate dl
rm /home/deli/ftp/ffn-c/results/fib25/training2/counters.txt
rm /home/deli/ftp/ffn-c/results/fib25/training2/seg*
python run_inference.py --inference_request="$(cat configs/test.pbtxt)" --bounding_box 'start { x:0 y:0 z:0 } size { x:300 y:300 z:200 }'
