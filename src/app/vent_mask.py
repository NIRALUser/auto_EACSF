#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon, Arthur Le Maout (alemaout@email.unc.edu)
######################################################################################################### 
import sys
import os
import argparse
import subprocess
import itk

def eprint(*args, **kwargs):
    #print errors function
    print(*args, file=sys.stderr, **kwargs)

def call_and_print(args):
    #external process calling function with output and errors printing
    print(">>>ARGS: "+"\n\t".join(args)+'\n')
    sys.stdout.flush()
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    out=out.decode('utf-8')
    err=err.decode('utf-8')
    if(out!=''):
        print(out+"\n")
        sys.stdout.flush()
    if(err!=''):
        eprint(err+'\n')
        sys.stderr.flush()
        print('\n'+args[0]+' : errors occured, see errors log for more details\n\n')
        sys.stdout.flush()
    else:
        print('\n'+args[0]+' : exit with success\n\n')
        sys.stdout.flush()

def main(args):
    sys.stdout.flush()
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

    ImageMath = args.ImageMath
    ANTS = args.ANTS
    WarpImageMultiTransform = args.WarpImageMultiTransform

    TISSUE_SEG = args.tissueSeg
    VENT_MASK = args.ventricleMask
    OUTPUT_DIR = args.output
    if not os.path.exists(OUTPUT_DIR):
        os.makedirs(OUTPUT_DIR)

    AUTO_EACSF_PARENT_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(OUTPUT_DIR))))
    MASKS_DIR = os.path.join(AUTO_EACSF_PARENT_DIR,'auto_EACSF/data/masks')
    VENT_REGION_MASK = os.path.join(MASKS_DIR,'Vent_CSF-BIN-RAI-Fusion_INV.nrrd')

    VENT_MASK_base=os.path.splitext(os.path.basename(VENT_MASK))[0]
    VENT_MASK_INV = os.path.join(OUTPUT_DIR,VENT_MASK_base+'_INV.nrrd')

    T1_dir=os.path.dirname(T1)
    T1_base=os.path.splitext(os.path.basename(T1))[0]
    OUT_VENT_MASK = os.path.join(OUTPUT_DIR, "".join([T1_base,"_AtlasToVent.nrrd"]))
    SEG_WithoutVent = os.path.join(OUTPUT_DIR, "".join([T1_base,"_EMS_withoutVent.nrrd"]))
    ANTs_MATRIX_NAME=os.path.join(OUTPUT_DIR, T1_base)
    ANTs_WARP = os.path.join(OUTPUT_DIR, "".join([T1_base,"Warp.nii.gz"]))
    ANTs_AFFINE = os.path.join(OUTPUT_DIR, "".join([T1_base,"Affine.txt"]))

    args = [ANTS, '3', '-m',SIM_METRIC+'['+T1+','+ATLAS+','+T1_WEIGHT+','+SIM_PARAMETER+']', '-i',ITERATIONS,'-o',ANTs_MATRIX_NAME, '-t', 'SyN['+TRANSFORMATION_STEP+']', '-r', 'Gauss['+GAUSSIAN+',0]']
    call_and_print(args)

    args=[WarpImageMultiTransform, '3', VENT_REGION_MASK, OUT_VENT_REGION_MASK, ANTs_WARP, ANTs_AFFINE, '-R', T1, '--use-NN']
    call_and_print(args)

    args=[ImageMath, TISSUE_SEG, '-mul', OUT_VENT_REGION_MASK, '-outfile', SEG_WithoutVent]
    call_and_print(args)

    if (VENT_MASK != ""):
        VM_im=itk.imread(VENT_MASK)
        inv_filter=itk.InvertIntensityImageFilter.New(VM_im)
        itk.imwrite(inv_filter,VENT_MASK_INV)

        args=[WarpImageMultiTransform, '3', VENT_MASK_INV, OUT_VENT_MASK, ANTs_WARP, ANTs_AFFINE, '-R', T1, '--use-NN']
        call_and_print(args)

        args=[ImageMath, SEG_WithoutVent, '-mul', OUT_VENT_MASK, '-outfile', SEG_WithoutVent]
        call_and_print(args)




if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Calculates segmentation w/o ventricle mask. Computes deformation field with T1 vs ATLAS, applies warp to ventricle mask and masks tissue-seg')
    parser.add_argument('--t1', type=str, help='T1 Image to calculate deformation field against atlas', default="@T1IMG@")
    parser.add_argument('--atlas', nargs='?', type=str, help='Atlas image', const="@ATLAS@")
    parser.add_argument('--registrationType', type=str, help='ANTS Registration Type', default="@REG_TYPE@")
    parser.add_argument('--transformationStep', type=str, help='Diffeomorphic gradient descent step length', default="@TRANS_STEP@")
    parser.add_argument('--iterations', type=str, help='ANTS Iterations for diffeomorphism', default="@ITERATIONS@")
    parser.add_argument('--simMetric', type=str, help='ANTS Similarity Metric Type (CC, MI, MSQ)', default="@SIM_METRIC@")
    parser.add_argument('--simParameter', type=str, help='Region Radius for CC, number of bins for MI, etc.', default="@SIM_PARAMETER@")
    parser.add_argument('--gaussian', type=str, help='ANTS Gaussian Sigma', default="@GAUSSIAN@")
    parser.add_argument('--t1Weight', type=str, help='T1 Weight', default="@T1_WEIGHT@")
    parser.add_argument('--tissueSeg', type=str, help='Tissue Segmentation', default="@TISSUE_SEG@")
    parser.add_argument('--ventricleMask', type=str, help='Ventricle mask', default="@VENTRICLE_MASK@")
    parser.add_argument('--ImageMath', type=str, help='ImageMath executable path', default='@IMAGEMATH_PATH@')
    parser.add_argument('--ANTS', type=str, help='ANTS executable path', default='@ANTS_PATH@')
    parser.add_argument('--WarpImageMultiTransform', type=str, help='WarpImageMultiTransform executable path', default='@WIMT_PATH@')
    parser.add_argument('--output', nargs='?', type=str, help='Output directory', const="@OUTPUT_DIR@")
    parser.add_argument('--outputName', type=str, help='Output masked tissue-seg', default="@OUTPUT_MASK@")
    args = parser.parse_args()
    main(args)
