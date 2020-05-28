#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon, Arthur Le Maout, Martin Styner
######################################################################################################### 
import sys
import os
import argparse
import subprocess

from main_script import call_and_print

def main(args):
    sys.stdout.flush()
    T1 = args.t1
    TEMPLATE_T1_VENTRICLE = args.templateT1Ventricle
    TEMPLATE_INV_MASK_VENTRICLE = args.templateInvMaskVentricle

    SUBJECT_VENTRICLE_MASK = args.subjectVentricleMask
    SUBJECT_TISSUE_SEG = args.subjectTissueSeg

    #ANTS REGISTRATION PARAMETERS
    REGISTRATION_TYPE = args.registrationType
    TRANSFORMATION_STEP = args.transformationStep
    ITERATIONS = args.iterations
    SIM_METRIC = args.simMetric
    SIM_PARAMETER = args.simParameter
    GAUSSIAN = args.gaussian
    T1_WEIGHT = args.t1Weight
    #############################

    ImageMath = args.ImageMath
    ANTS = args.ANTS
    WarpImageMultiTransform = args.WarpImageMultiTransform

    OUTPUT_DIR = args.output
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    if (os.path.isfile(TEMPLATE_INV_MASK_VENTRICLE)):
        T1_dir=os.path.dirname(T1)
        T1_base = os.path.splitext(os.path.basename(T1))[0]
        TEMPLATE_INV_MASK_VENTRICLE_base = os.path.splitext(os.path.basename(TEMPLATE_INV_MASK_VENTRICLE))[0]
        TEMPLATE_INV_MASK_VENTRICLE_REG = os.path.join(OUTPUT_DIR, "".join([TEMPLATE_INV_MASK_VENTRICLE_base,"_Registered.nrrd"]))
        TEMPLATE_T1_VENTRICLE_base = os.path.splitext(os.path.basename(TEMPLATE_T1_VENTRICLE))[0]
        TEMPLATE_T1_VENTRICLE_REG = os.path.join(OUTPUT_DIR, "".join([TEMPLATE_T1_VENTRICLE_base,"_Registered.nrrd"]))
        SEG_WithoutVent = os.path.join(OUTPUT_DIR, "".join([T1_base,"_EMS_withoutVent.nrrd"]))
        ANTs_MATRIX_NAME=os.path.join(OUTPUT_DIR, T1_base)
        ANTs_WARP = os.path.join(OUTPUT_DIR, "".join([T1_base,"Warp.nii.gz"]))
        ANTs_INV_WARP = os.path.join(OUTPUT_DIR, "".join([T1_base,"Warp.nii.gz"]))
        ANTs_AFFINE = os.path.join(OUTPUT_DIR, "".join([T1_base,"Affine.txt"]))

        if not (os.path.isfile(ANTs_WARP) and os.path.isfile(ANTs_AFFINE)):
            args = [ANTS, '3', '-m',SIM_METRIC+'['+T1+','+TEMPLATE_T1_VENTRICLE+','+T1_WEIGHT+','+SIM_PARAMETER+']', '-i',ITERATIONS,'-o',ANTs_MATRIX_NAME, '-t', 'SyN['+TRANSFORMATION_STEP+']', '-r', 'Gauss['+GAUSSIAN+',0]']
            call_and_print(args)
        else:
            print('Ventricle masking : ANTs registration already exists')

        args=[WarpImageMultiTransform, '3', TEMPLATE_INV_MASK_VENTRICLE, TEMPLATE_INV_MASK_VENTRICLE_REG, ANTs_WARP, ANTs_AFFINE, '-R', T1, '--use-NN']
        call_and_print(args)

        args=[WarpImageMultiTransform, '3', TEMPLATE_T1_VENTRICLE, TEMPLATE_T1_VENTRICLE_REG, ANTs_WARP, ANTs_AFFINE, '-R', T1, '--use-NN']
        call_and_print(args)

        if SUBJECT_TISSUE_SEG is not None and os.path.exists(SUBJECT_TISSUE_SEG):
            args=[ImageMath, SUBJECT_TISSUE_SEG, '-mul', TEMPLATE_INV_MASK_VENTRICLE_REG, '-outfile', SEG_WithoutVent]
            call_and_print(args)
    else:
        print("Template inverse ventricle mask is not a file and cannot be applied")

    if (os.path.isfile(SUBJECT_VENTRICLE_MASK) and TEMPLATE_INV_MASK_VENTRICLE == "" ):
        SUBJECT_VENTRICLE_MASK_base=os.path.splitext(os.path.basename(SUBJECT_VENTRICLE_MASK))[0]
        SUBJECT_VENTRICLE_MASK_INV = os.path.join(OUTPUT_DIR,SUBJECT_VENTRICLE_MASK_base+'_INV.nrrd')

        args = [ImageMath, SUBJECT_VENTRICLE_MASK, '-threshold', '0,0', '-outfile', SUBJECT_VENTRICLE_MASK_INV]
        call_and_print(args)

        SEG_WithoutVent = os.path.join(OUTPUT_DIR, "".join([T1_base,"_EMS_withoutVent.nrrd"]))
        
        args=[ImageMath, SUBJECT_TISSUE_SEG, '-mul', SUBJECT_VENTRICLE_MASK_INV, '-outfile', SEG_WithoutVent]
        call_and_print(args)


if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Calculates segmentation w/o ventricle mask. Computes deformation field with T1 vs ATLAS, applies warp to ventricle mask and masks tissue-seg')
    parser.add_argument('--t1', type=str, help='T1 Image to calculate deformation field against atlas', default="@T1IMG@")
    parser.add_argument('--templateT1Ventricle', type=str, help='Atlas template T1', default="@TEMPT1VENT@")
    parser.add_argument('--templateInvMaskVentricle', type=str, help='Atlas ventricle mask', default="@TEMP_INV_VMASK@")
    parser.add_argument('--subjectVentricleMask', type=str, help='Subject specific ventricle mask', default="@SUB_VMASK@")
    parser.add_argument('--subjectTissueSeg', type=str, help='Tissue Segmentation', default="@TISSUE_SEG@")
    parser.add_argument('--registrationType', type=str, help='ANTS Registration Type', default="@REG_TYPE@")
    parser.add_argument('--transformationStep', type=str, help='Diffeomorphic gradient descent step length', default="@TRANS_STEP@")
    parser.add_argument('--iterations', type=str, help='ANTS Iterations for diffeomorphism', default="@ITERATIONS@")
    parser.add_argument('--simMetric', type=str, help='ANTS Similarity Metric Type (CC, MI, MSQ)', default="@SIM_METRIC@")
    parser.add_argument('--simParameter', type=str, help='Region Radius for CC, number of bins for MI, etc.', default="@SIM_PARAMETER@")
    parser.add_argument('--gaussian', type=str, help='ANTS Gaussian Sigma', default="@GAUSSIAN@")
    parser.add_argument('--t1Weight', type=str, help='T1 Weight', default="@T1_WEIGHT@")
    parser.add_argument('--ImageMath', type=str, help='ImageMath executable path', default='@ImageMath_PATH@')
    parser.add_argument('--ANTS', type=str, help='ANTS executable path', default='@ANTS_PATH@')
    parser.add_argument('--WarpImageMultiTransform', type=str, help='WarpImageMultiTransform executable path', default='@WarpImageMultiTransform_PATH@')
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    #parser.add_argument('--outputName', type=str, help='Output masked tissue-seg', default="@OUTPUT_MASK@")
    args = parser.parse_args()
    main(args)
