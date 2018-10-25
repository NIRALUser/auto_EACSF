#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon (email: hanbit.yoon@gmail.com)
#########################################################################################################
from __future__ import print_function
import argparse
import os
import sys
import subprocess
from subprocess import call
import itk
import numpy as np

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
    print("Running main_script")
    sys.stdout.flush()

    ### Coronal mask settings
    ACPC_unit=args.ACPCunit
    if(ACPC_unit == "index"):
        ACPC_val=int(args.ACPCval)
    else:
        ACPC_mm=float(args.ACPCval)
        im=itk.imread("/NIRAL/work/alemaout/sources/Projects/auto_EACSF-Project/auto_EACSF/data/stx_noscale_718312_V24_t1w_RAI.nrrd")
        index_coord=im.TransformPhysicalPointToContinuousIndex([ACPC_mm,0,0])
        ACPC_val=round(index_coord[0])

    if(ACPC_val == 70):
        Coronal_Mask = "/work/alemaout/sources/Projects/auto_EACSF-Project/auto_EACSF/data/Coronal_70Mask.nrrd"
    else:
        im=itk.imread("/NIRAL/work/alemaout/sources/Projects/auto_EACSF-Project/auto_EACSF/data/stx_noscale_718312_V24_t1w_RAI.nrrd")
        np_copy=itk.GetArrayFromImage(im)
        if ((ACPC_val >= np_copy.shape[0]) | (ACPC_val <= 0)):
            eprint("ACPC index out of range ("+str(ACPC_val)+"), using default coronal mask (slice 70)")
            sys.stderr.flush()
            Coronal_Mask = "/work/alemaout/sources/Projects/auto_EACSF-Project/auto_EACSF/data/Coronal_70Mask.nrrd"
        else:
            np_copy[ACPC_val-1:np_copy.shape[0]-1,:,:]=np.iinfo(np_copy.dtype).max
            np_copy[0:ACPC_val-1,:,:]=0
            itk_np_copy=itk.GetImageViewFromArray(np_copy)
            itk_outfile_name="Coronal_"+str(ACPC_val)+"Mask.nrrd"
            itk.imwrite(itk_np_copy,itk_outfile_name)
            Coronal_Mask = itk_outfile_name

    ### Inputs
    T1 = args.t1
    T2 = args.t2
    T2_exists=True
    if (T2 == ""):
        T2_exists=False

    T1_split=os.path.splitext(os.path.basename(T1))
    if (T1_split[1] == 'gz'):
        T1_base=os.path.splitext(T1_split[0])
    else:
        T1_base=T1_split[0]

    if (T2_exists):
        T2_split=os.path.splitext(os.path.basename(T2))
        if (T2_split[1] == 'gz'):
            T2_base=os.path.splitext(T2_split[0])
        else:
            T2_base=T2_split[0]

    ### Executables
    python=args.python3
    ImageMath=args.ImageMath
    ImageStat=args.ImageStat
    ABC=args.ABC

    ### Masks
    BRAIN_MASK = args.brainMask

    if (args.useDfCerMask == "true"):
        PRE_CEREBELLUM_MASK = "/work/alemaout/sources/Projects/auto_EACSF-Project/auto_EACSF/data/CVS_MASK_RAI_Dilate.nrrd"
    else:
        PRE_CEREBELLUM_MASK = args.cerebellumMask
    Segmentation = args.tissueSeg

    ### Output path
    OUT_PATH = args.output

    if (args.performReg == "true"):
       print("######## Running rigid_align_script ########")
       sys.stdout.flush()
       OUT_RR=os.path.join(OUT_PATH,'RigidRegistration')
       call([python, "rigid_align_script.py",'--output',OUT_RR])
       T1 = os.path.join(OUT_RR, "".join([T1_base,"_stx.nrrd"]))
       if (T2_exists):
           T2=os.path.join(OUT_RR, "".join([T2_base,"_stx.nrrd"]))

       print("######## Finished running rigid_align_script ########")
       sys.stdout.flush()

    if (args.performSS == "true"):
       print("######## Running make_mask_script ########")
       sys.stdout.flush()
       OUT_SS=os.path.join(OUT_PATH,'SkullStripping')
       args=[python, "make_mask_script.py", '--t1', T1, '--t2', T2, '--at_dir', '--at_list', '--output',OUT_SS]
       print(args)
       call(args)
       BRAIN_MASK = os.path.join(OUT_SS, "".join([T1_base,"_FinalBrainMask.nrrd"]))
       print("######## Finished running make_mask_script ########")
       sys.stdout.flush()


    if (args.performTSeg == "true"):
       print("######## Running ABC Segmentation ########")
       sys.stdout.flush()
       args=[ABC, 'ABCparam.xml']
       call(args)
       print("######## Finished running ABC Segmentation ########")
       sys.stdout.flush()

    if (args.performVR == "true"):
       print("######## Running vent_mask_script ########")
       sys.stdout.flush()
       OUT_VR=os.path.join(OUT_PATH,'VentricleMasking')
       call([python, "vent_mask_script.py", '--t1', T1, '--output',OUT_VR])
       Segmentation = os.path.join(OUT_VR, "".join([T1_base,"_EMS_withoutVent.nrrd"]))
       print("######## Finished running vent_mask_script ########")
       sys.stdout.flush()

    BRAIN_MASK_dir = os.path.dirname(BRAIN_MASK)
    BRAIN_MASK_base = os.path.splitext(os.path.basename(BRAIN_MASK))[0]
    PRE_CEREBELLUM_MASK_dir = os.path.dirname(PRE_CEREBELLUM_MASK)
    PRE_CEREBELLUM_MASK_base = os.path.splitext(os.path.basename(PRE_CEREBELLUM_MASK))[0]
    Segmentation_dir = os.path.dirname(Segmentation)
    Segmentation_base = os.path.splitext(os.path.basename(Segmentation))[0]

    ######### Cutting lateral ventricle ######
    print("#### Cutting lateral ventricle ####")
    MID_TEMP00 = os.path.join(OUT_PATH, "".join([T1_base,"_MID00.nrrd"]))
    args=[ImageMath, Segmentation, '-outfile', MID_TEMP00, '-mul', BRAIN_MASK]
    call_and_print(args)

    ######### Cutting below AC-PC line #######
    print("#### Cutting below AC-PC line ####")
    MID_TEMP01 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID01.nrrd"]))
    MID_TEMP02 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID02.nrrd"]))
    MID_TEMP03 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID03.nrrd"]))

    args=[ImageMath, MID_TEMP00, '-outfile', MID_TEMP01, '-mul', Coronal_Mask]
    call_and_print(args)

    args=[ImageMath, MID_TEMP01, '-outfile', MID_TEMP02, '-extractLabel', '3']
    call_and_print(args)

    args=[ImageMath, MID_TEMP02, '-outfile', MID_TEMP02, '-sub', PRE_CEREBELLUM_MASK]
    call_and_print(args)

    args=[ImageMath, MID_TEMP02, '-outfile', MID_TEMP02, '-threshold', '1,1']
    call_and_print(args)

    args=[ImageMath, MID_TEMP02, '-outfile', MID_TEMP03, '-conComp','1']
    call_and_print(args)

    ######### Make thin mask for preserving outter 2nd or 3rd fracrtion csf ##########
    print("#### Make thin mask for preserving outter 2nd or 3rd fracrtion csf ####")
    BRAIN_MASK = os.path.join(OUT_PATH,'Brain_Mask.nrrd')
    BRAIN_MASK_ERODE = os.path.join(OUT_PATH,'Brain_Mask_Erode.nrrd')
    THIN_MASK = os.path.join(OUT_PATH,'Brain_Thin_Mask.nrrd')

    args=[ImageMath, Segmentation, '-threshold', '1,4', '-outfile', BRAIN_MASK]
    call_and_print(args)

    args=[ImageMath, BRAIN_MASK, '-erode', '6,1', '-outfile', BRAIN_MASK_ERODE]
    call_and_print(args)

    args=[ImageMath, BRAIN_MASK, '-sub', BRAIN_MASK_ERODE, '-outfile', THIN_MASK]
    call_and_print(args)

    args=[ImageMath, THIN_MASK, '-mul', Coronal_Mask, '-outfile', THIN_MASK]
    call_and_print(args)

    ########## Find outer CSF in thin mask and added this csf to extra CSF #############
    print("#### Find outer CSF in thin mask and added this csf to extra CSF ####")
    PRESERVED_OUTERCSF = os.path.join(OUT_PATH,'Preserved_OuterCSF.nrrd')
    FINAL_OUTERCSF = os.path.join(OUT_PATH, "".join([Segmentation_base,"_extCSF.nrrd"]))
    FINAL_OUTERCSF_dir = os.path.dirname(FINAL_OUTERCSF)
    FINAL_OUTERCSF_base = os.path.splitext(os.path.basename(FINAL_OUTERCSF))[0]

    args=[ImageMath, MID_TEMP02, '-outfile', PRESERVED_OUTERCSF, '-mul', THIN_MASK]
    call_and_print(args)

    args=[ImageMath, MID_TEMP03, '-outfile', FINAL_OUTERCSF, '-add', PRESERVED_OUTERCSF]
    call_and_print(args)

    args=[ImageMath, FINAL_OUTERCSF, '-outfile', FINAL_OUTERCSF, '-threshold', '1,2']
    call_and_print(args)

    args=[ImageStat, FINAL_OUTERCSF, '-label', FINAL_OUTERCSF]
    call_and_print(args)

    print("######## Auto_EACSF finished ########")

    sys.exit(0);

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Calculates outter CSF')
    parser.add_argument('--t1', type=str, help='T1 Image', default="@T1IMG@")
    parser.add_argument('--t2', type=str, help='T2 Image', default="@T2IMG@")
    parser.add_argument('--brainMask', type=str, help='Brain mask', default="@BRAIN_MASK@")
    parser.add_argument('--cerebellumMask', type=str, help='Cereb Mask', default="@CEREB_MASK@")
    parser.add_argument('--tissueSeg', type=str, help='Tissue Segmentation', default="@TISSUE_SEG@")
    parser.add_argument('--ACPCunit', type=str, help='ACPC unit (mm/index)', default="@ACPC_UNIT@")
    parser.add_argument('--ACPCval', type=str, help='ACPC value', default="@ACPC_VAL@")
    parser.add_argument('--useDfCerMask', type=str, help='Use the default cerebellum mask', default="@USE_DCM@")
    parser.add_argument('--performReg', type=str, help='Perform rigid registration', default="@PERFORM_REG@")
    parser.add_argument('--performSS', type=str, help='Perform skull stripping', default="@PERFORM_SS@")
    parser.add_argument('--performTSeg', type=str, help='Perform tissue segmentation', default="@PERFORM_TSEG@")
    parser.add_argument('--performVR', type=str, help='Perform ventricle removal', default="@PERFORM_VR@")
    parser.add_argument('--python3', type=str, help='Python3 executable path', default='@PY3_PATH@')
    parser.add_argument('--ImageMath', type=str, help='ImageMath executable path', default='@IMAGEMATH_PATH@')
    parser.add_argument('--ImageStat', type=str, help='ImageStat executable path', default='@IMAGESTAT_PATH@')
    parser.add_argument('--ABC', type=str, help='ABC executable path', default='@ABC_PATH@')
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)
