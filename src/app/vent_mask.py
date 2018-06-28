#!/tools/Python/Python-2.7.3/bin/python2.7
##	by Han Bit Yoon (email: hanbit.yoon@gmail.com)
######################################################################################################### 
import sys
import os
import argparse
import subprocess

def main(args):

    T1 = args.t1
    ATLAS = args.atlas

    #ANTS REGISTRATION PARAMETERS
    REGISTRATION_TYPE = args.registrationType
    TRANSFORMATION_STEP = args.transformationStep
    ITERATIONS = args.iterations
    SIM_METRIC = args.simMetric
    SIM_PARAMETER = args.simParameter
    GAUSSIAN = args.gaussian
    T1_WEIGHT = args.t1Weight
    #############################

    TISSUE_SEG = args.tissueSeg
    VENT_MASK = args.ventricleMask
    OUTPUT_DIR = args.output

    T1_dir=os.path.dirname(T1)
    T1_base=os.path.splitext(os.path.basename(T1))[0]
    OUT_VENT_MASK = os.path.join(OUTPUT_DIR, "".join([T1_base,"_AtlasToVent.nrrd"]))
    SEG_WithoutVent = os.path.join(OUTPUT_DIR, "".join([T1_base,"_EMS_withoutVent.nrrd"]))
    ANTs_MATRIX_NAME=os.path.join(OUTPUT_DIR, T1_base)
    ANTs_WARP = os.path.join(OUTPUT_DIR, "".join([T1_base,"_Warp.nii.gz"]))
    ANTs_AFFINE = os.path.join(OUTPUT_DIR, "".join([T1_base,"_Affine.txt"]))

    args = 'ANTS 3 -m '+SIM_METRIC+'\\[%s, %s,'+T1_WEIGHT+','+SIM_PARAMETER+'\\] -i '+ITERATIONS+' -o %s -t SyN\\['+TRANSFORMATION_STEP+'\\] -r Gauss\\['+GAUSSIAN+',0\\]'
    os.system(args %(T1, ATLAS, ANTs_MATRIX_NAME) )
    #os.system('ANTS 3 -m CC\\[%s, %s,1,4\\] -i 100x50x25 -o %s -t SyN\\[0.25\\] -r Gauss\\[3,0\\]' %(T1, ATLAS, ANTs_MATRIX_NAME) )
    
    #args_1 ='CC\\['+T1+','+ATLAS+',1,4\\]'
    #print args_1
    #args=['ANTS', '3', '-m', args_1, '-i', '100x50x25', '-o', ANTs_MATRIX_NAME, '-t', 'SyN\\[0.25\\]', '-r', 'Gauss\\[3,0\\]']
    #ants = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    #out, err = ants.communicate()

    args=['WarpImageMultiTransform', '3', VENT_MASK, OUT_VENT_MASK, ANTs_WARP, ANTs_AFFINE, '-R', T1, '--use-NN']
    warpImg = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = warpImg.communicate()

    args=['ImageMath', TISSUE_SEG, '-mul', OUT_VENT_MASK, '-outfile', SEG_WithoutVent]
    ImgMath = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = ImgMath.communicate()

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Calculates segmentation w/o ventricle mask. Computes deformation field with T1 vs ATLAS, applies warp to ventricle mask and masks tissue-seg')
    parser.add_argument('--t1', type=str, help='T1 Image to calculate deformation field against atlas', default="@T1IMG@")
    parser.add_argument('--atlas', type=str, help='Atlas image', default="@ATLAS@")
    parser.add_argument('--registrationType', type=str, help='ANTS Registration Type', default="@REG_TYPE@")
    parser.add_argument('--transformationStep', type=str, help='Diffeomorphic gradient descent step length', default="@TRANS_STEP@")
    parser.add_argument('--iterations', type=str, help='ANTS Iterations for diffeomorphism', default="@ITERATIONS@")
    parser.add_argument('--simMetric', type=str, help='ANTS Similarity Metric Type (CC, MI, MSQ)', default="@SIM_METRIC@")
    parser.add_argument('--simParameter', type=str, help='Region Radius for CC, number of bins for MI, etc.', default="@SIM_PARAMETER@")
    parser.add_argument('--gaussian', type=str, help='ANTS Gaussian Sigma', default="@GAUSSIAN@")
    parser.add_argument('--t1Weight', type=str, help='T1 Weight', default="@T1_WEIGHT@")
    parser.add_argument('--tissueSeg', type=str, help='Tissue Segmentation', default="@TISSUE_SEG@")
    parser.add_argument('--ventricleMask', type=str, help='Ventricle mask', default="@VENTRICLE_MASK@")
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    parser.add_argument('--outputName', type=str, help='Output masked tissue-seg', default="@OUTPUT_MASK@")
    args = parser.parse_args()
    main(args)
