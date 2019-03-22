#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon, Arthur Le Maout (alemaout@email.unc.edu)
#########################################################################################################
from __future__ import print_function
import argparse
import os
import sys
import subprocess
from subprocess import call
import itk
import numpy as np

def print_main_info(info):
    info="<b>>>> "+os.path.basename(sys.argv[0])+' : <font color="blue">'+info+"</font></b>"
    print(info)
    sys.stdout.flush()

def print_aef(info):
    #print already existing file infos
    info="<b>>>> "+os.path.basename(sys.argv[0])+' : <font color="yellow">'+info+"</font></b>"
    sys.stdout.flush()

def eprint(*args, **kwargs):
    #print errors function
    print(*args, file=sys.stderr, **kwargs)

def call_and_print(args):
    #external process calling function with output and errors printing
    exe_path=args.pop(0)
    exe_dir=os.path.dirname(exe_path)
    exe_name=os.path.basename(exe_path)
    print("<b>   >>> "+os.path.basename(sys.argv[0])+' : <font color="blue">Running: </font></b>'+exe_dir+'/<b><font color="blue">'+exe_name+'</font></b> '+" ".join(args)+'\n')
    sys.stdout.flush()
    args.insert(0,exe_path)
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    out, err = proc.communicate()
    out=out.decode('utf-8')
    err=err.decode('utf-8')
    if(out!=''):
        print(out+"\n")
        sys.stdout.flush()
    if(err!=''):
        eprint('<font color="red"><b>While running : </b></font>'+exe_dir+'/<b><font color="red">'+exe_name+'</font></b> '+" ".join(args)+'\n')
        eprint('<font color="red"><b>Following error(s) occured : </b></font>\n')
        eprint(err+'\n')
        sys.stderr.flush()
        print('\n<font color="blue"><b>'+args[0]+' :</b></font> <font color="red">errors occured, see errors log for more details</font>\n\n')
        sys.stdout.flush()
    else:
        print('\n<font color="blue"><b>'+args[0]+' :</b></font> <font color="green">exit with success</font>\n\n')
        sys.stdout.flush()

def main(main_args):
    print("<b>>>> Running "+os.path.basename(sys.argv[0])+"</b>\n")
    sys.stdout.flush()

    ### Inputs
    T1 = main_args.t1
    T2 = main_args.t2
    T2_exists = True
    if (T2 == ""):
        T2_exists = False

    ### Executables
    python = main_args.python3
    ImageMath = main_args.ImageMath
    ImageStat = main_args.ImageStat
    ABC = main_args.ABC

    ### Masks
    CSFLabel = int(main_args.CSFLabel)

    ### Output path
    OUT_PATH = main_args.output

    ### Scripts
    scripts_prefix = os.path.join(OUT_PATH, "PythonScripts")

    if (main_args.registration == "true"):
        rigid_align_script = os.path.join(scripts_prefix, "rigid_align_script.py")
        print_main_info("Running " + rigid_align_script)

        OUT_RR = os.path.join(OUT_PATH,'RigidRegistration')
        call([python, rigid_align_script, '--output', OUT_RR])
        T1 = os.path.join(OUT_RR, "".join([T1_base,"_stx.nrrd"]))
        if (T2_exists):
           T2 = os.path.join(OUT_RR, "".join([T2_base,"_stx.nrrd"]))

        print_main_info("Finished running "+rigid_align_script)

    T1_split=os.path.splitext(os.path.basename(T1))
    if (T1_split[1] == 'gz'):
        T1_base = os.path.splitext(T1_split[0])
    else:
        T1_base = T1_split[0]

    if (T2_exists):
        T2_split = os.path.splitext(os.path.basename(T2))
        if (T2_split[1] == 'gz'):
            T2_base = os.path.splitext(T2_split[0])
        else:
            T2_base = T2_split[0]

    if (main_args.skullStripping == "true"):
        make_mask_script = os.path.join(scripts_prefix, "make_mask_script.py")
        print_main_info("Running " + make_mask_script)

        OUT_SS = os.path.join(OUT_PATH, 'SkullStripping')
        BRAIN_MASK = os.path.join(OUT_SS, "".join([T1_base,"_FinalBrainMask.nrrd"]))
        if not(os.path.isfile(BRAIN_MASK)):
           call([python, make_mask_script, '--t1', T1, '--t2', T2, '--at_dir', '--at_list', '--output',OUT_SS])
        else:
           print('Brainmask already exists')
        print_main_info("Finished running " + make_mask_script)
    else:
        BRAIN_MASK = main_args.brainMask


    if (main_args.segmentation == "true"):
        tissue_seg_script = os.path.join(scripts_prefix, "tissue_seg_script.py")
        print_main_info("Running " + tissue_seg_script)

        OUT_TS=os.path.join(OUT_PATH,'TissueSegAtlas')
        OUT_ABC=os.path.join(OUT_PATH,'ABC_Segmentation')
        Segmentation = os.path.join(OUT_ABC, "".join([T1_base,"_labels_EMS.nrrd"]))
        print(Segmentation)
        if not(os.path.isfile(Segmentation)):
           print(Segmentation + ' doesnt exist')
           call([python, tissue_seg_script, '--t1', T1, '--t2', T2,'--at_dir', '--output', OUT_TS])
        else:
           print('Segmentation already exists')
        print_main_info("Finished running tissue_seg_script.py")

    else:
        Segmentation = main_args.tissueSeg

    if (main_args.removeVentricles == "true"):
        vent_mask_script = os.path.join(scripts_prefix, "vent_mask_script.py")
        print_main_info("Running " + vent_mask_script)
        OUT_VR=os.path.join(OUT_PATH,'VentricleMasking')
        call([python, vent_mask_script, '--t1', T1, '--subjectTissueSeg', Segmentation, '--output',OUT_VR])
        Segmentation = os.path.join(OUT_VR, "".join([T1_base,"_EMS_withoutVent.nrrd"]))
        print_main_info("Finished running "+vent_mask_script)

    BRAIN_MASK_base = os.path.splitext(os.path.basename(BRAIN_MASK))[0]
    Segmentation_base = os.path.splitext(os.path.basename(Segmentation))[0]

    ######### Stripping the skull ######
    print_main_info("Stripping the skull")
    MID_TEMP00 = os.path.join(OUT_PATH, "".join([T1_base,"_MID00.nrrd"]))
    args=[ImageMath, Segmentation, '-outfile', MID_TEMP00, '-mul', BRAIN_MASK]
    call_and_print(args)

    ######### Cutting below AC-PC line #######
    ### Coronal mask creation
    print_main_info("Cutting below AC-PC line")

    ACPC_unit=main_args.ACPCunit
    if(ACPC_unit == "index"):
        ACPC_val=int(main_args.ACPCval)
    else:
        ACPC_mm=float(main_args.ACPCval)
        im=itk.imread(T1)
        index_coord=im.TransformPhysicalPointToContinuousIndex([ACPC_mm,0,0])
        ACPC_val=round(index_coord[0])

    Coronal_Mask = "coronal_mask_"+str(ACPC_val)+".nrrd"
    if not (os.path.isfile(Coronal_Mask)):
        im=itk.imread(T1)
        np_copy=itk.GetArrayFromImage(im)
        if ((ACPC_val >= np_copy.shape[0]) | (ACPC_val <= 0)):
            eprint("ACPC index out of range ("+str(ACPC_val)+"), using default coronal mask (slice 70)")
            sys.stderr.flush()
            ACPC_val = 70;

        print_main_info('Creating coronal mask')
        np_copy[ACPC_val-1:np_copy.shape[0]-1,:,:]=1
        np_copy[0:ACPC_val-1,:,:]=0
        itk_np_copy=itk.GetImageViewFromArray(np_copy)
        itk_np_copy.SetOrigin(im.GetOrigin())
        itk_np_copy.SetSpacing(im.GetSpacing())
        itk_np_copy.SetDirection(im.GetDirection())
        itk.imwrite(itk_np_copy,Coronal_Mask)
        print_main_info('Coronal mask created')
    else:
        print_main_info('Loading ' + Coronal_Mask)

    ### Mask multiplication
    MID_TEMP01 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID01.nrrd"]))
    MID_TEMP02 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID02.nrrd"]))
    MID_TEMP03 = os.path.join(OUT_PATH,"".join([Segmentation_base,"_MID03.nrrd"]))

    args=[ImageMath, MID_TEMP00, '-outfile', MID_TEMP01, '-mask', Coronal_Mask]
    call_and_print(args)

    args=[ImageMath, MID_TEMP01, '-outfile', MID_TEMP02, '-extractLabel', '3']
    call_and_print(args)

    args=[ImageMath, MID_TEMP02, '-outfile', MID_TEMP03, '-conComp','1']
    call_and_print(args)

    MID_TEMP02_base = os.path.splitext(os.path.basename(MID_TEMP02))[0]
    MID_TEMP03_base = os.path.splitext(os.path.basename(MID_TEMP03))[0]

    MINC_MID_TEMP02 = MID_TEMP02_base + ".mnc"
    MINC_MID_TEMP03 = MID_TEMP03_base + ".mnc"
    MINC_BRAIN_MASK = BRAIN_MASK_base + ".mnc"
    MINC_Coronal_Mask = Coronal_Mask[:-5] + ".mnc"

    os.system("itk_convert %s %s" %(MID_TEMP03,MINC_MID_TEMP03))
    os.system("itk_convert %s %s" %(MID_TEMP02,MINC_MID_TEMP02))
    os.system("itk_convert %s %s" %(BRAIN_MASK,MINC_BRAIN_MASK))
    os.system("itk_convert %s %s" %(Coronal_Mask,MINC_Coronal_Mask))

    Erosion_Mask = BRAIN_MASK_base + "_Erosion.mnc"
    os.system("mincmorph -clobber -erosion %s %s" %(MINC_BRAIN_MASK,Erosion_Mask))
    for i in range(0,20):
            os.system("mincmorph -clobber -erosion %s %s" %(Erosion_Mask,Erosion_Mask))

    MINC_FINAL_CSF_SEG = Segmentation_base + "_FINAL_Partial_CSF.mnc"
    FINAL_CSF_SEG = MINC_FINAL_CSF_SEG[:-4]+".nrrd"
    os.system("minccalc -byte -expr 'if(A[0]==0 && A[1]==1 && A[2]==1){out=1}else{out=A[3]}' %s %s %s %s %s" %(Erosion_Mask,MINC_Coronal_Mask,MINC_MID_TEMP02,MINC_MID_TEMP03,MINC_FINAL_CSF_SEG))

    ## add script erase quad....
    Erosion_MINC = MINC_FINAL_CSF_SEG[:-4] + "_Ero.mnc"
    Erosion_MINC_MASKING = Erosion_MINC[:-4] + "_Masking.mnc"
    os.system("mincmorph -erosion %s %s" %(MINC_FINAL_CSF_SEG,Erosion_MINC))

#    for j in range(0,20):
#            os.system("mincmorph -clobber -erosion %s %s" %(Erosion_Mask,Erosion_Mask))

    os.system("minccalc -byte -expr 'if(A[0]>0){out=A[1]}else{out=0}' %s %s %s" %(Erosion_Mask,Erosion_MINC,Erosion_MINC_MASKING))

    Erosion_MINC_MASKING_NRRD = Erosion_MINC_MASKING[:-4] + ".nrrd"
    #os.system("convertITKformats2 %s %s" %(Erosion_MINC_MASKING,Erosion_MINC_MASKING_NRRD))
    os.system("itk_convert %s %s" %(Erosion_MINC_MASKING,Erosion_MINC_MASKING_NRRD))

    Erosion_MINC_MASKING_NRRD_conComp = Erosion_MINC_MASKING_NRRD[:-5] + "_conComp.nrrd"
    os.system("ImageMath %s -outfile %s -conComp 1" %(Erosion_MINC_MASKING_NRRD,Erosion_MINC_MASKING_NRRD_conComp))

    Erosion_MINC_MASKING_MINC_conComp=Erosion_MINC_MASKING_NRRD_conComp[:-5] + ".mnc"
    #os.system("convertITKformats2 %s %s" %(Erosion_MINC_MASKING_NRRD_conComp,Erosion_MINC_MASKING_MINC_conComp))
    os.system("itk_convert %s %s" %(Erosion_MINC_MASKING_NRRD_conComp,Erosion_MINC_MASKING_MINC_conComp))

    Dilation_comComp = Erosion_MINC_MASKING_MINC_conComp[:-4]+"_dil.mnc"
    os.system("mincmorph -clobber -dilation %s %s" %(Erosion_MINC_MASKING_MINC_conComp,Dilation_comComp))
    for k in range(0,8):
            os.system("mincmorph -clobber -dilation %s %s" %(Dilation_comComp,Dilation_comComp))
    FIANL_RESULT = MINC_FINAL_CSF_SEG[:-4] + "_QCistern.mnc"
    os.system("minccalc -byte -expr 'if(A[0]>0){out=0}else{out=A[1]}' %s %s %s" %(Dilation_comComp,MINC_FINAL_CSF_SEG,FIANL_RESULT))

    FINAL_RESULT_NRRD = FIANL_RESULT[:-4] + ".nrrd"
    FINAL_RESULT_NII = FIANL_RESULT[:-4] + ".nii.gz"
    #os.system("convertITKformats2 %s %s" %(FIANL_RESULT,FINAL_RESULT_NRRD))
    #os.system("convertITKformats2 %s %s" %(FIANL_RESULT,FINAL_RESULT_NII))
    os.system("itk_convert %s %s" %(FIANL_RESULT,FINAL_RESULT_NRRD))

    FINAL_RESULT_DILAT = FINAL_RESULT_NRRD[:-5] + '--DIL.nrrd'
    FINAL_RESULT_ERODE = FINAL_RESULT_DILAT[:-5] + '--ERO.nrrd'
    os.system("ImageMath %s -dilate 1,1 -outfile %s" %(FINAL_RESULT_NRRD,FINAL_RESULT_DILAT) )
    os.system("ImageMath %s -erode 1,1 -outfile %s" %(FINAL_RESULT_DILAT,FINAL_RESULT_ERODE) )

    #FINAL_TEMP04 = Segmenataion[:-5] + "_MID4.nrrd"
    #FINAL_TEMP05 = Segmenataion[:-5] + "_MID5.nrrd"
    #os.system("ImageMath %s -add %s -outfile %s" %(MID_TEMP03,FINAL_RESULT_ERODE,FINAL_TEMP04) )
    #os.system("ImageMath %s -threshold 1,2 -outfile %s " %(FINAL_TEMP04,FINAL_TEMP05) )

    #os.system("ImageMath %s -outfile %s -conComp 1" %(FINAL_RESULT_ERODE,FINAL_RESULT_ERODE))
    os.system("itk_convert %s %s" %(FINAL_RESULT_ERODE,FINAL_RESULT_NII))


    if (main_args.computeCSFDensity == "true"):
        LH_INNER_SURF = main_args.LHInnerSurf
        RH_INNER_SURF = main_args.RHInnerSurf
        OUT_CCD=os.path.join(OUT_PATH,'CSF_Density')
        args = ['computecsfdensity', '-l', LH_INNER_SURF, '-r', RH_INNER_SURF, '-s', Segmentation, '-c', FINAL_OUTERCSF, '-o', OUT_CCD, '-p', Segmentation_base]
        call_and_print(args)

    print_main_info("Auto_EACSF finished")

    sys.exit(0);

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Calculates outter CSF')
    parser.add_argument('--t1', type=str, help='T1 Image', default="@T1IMG@")
    parser.add_argument('--t2', type=str, help='T2 Image', default="@T2IMG@")
    parser.add_argument('--brainMask', type=str, help='Brain mask', default="@BRAIN_MASK@")
    parser.add_argument('--cerebellumMask', type=str, help='Cereb Mask', default="@CEREB_MASK@")
    parser.add_argument('--tissueSeg', type=str, help='Tissue Segmentation', default="@TISSUE_SEG@")
    parser.add_argument('--CSFLabel', type=str, help='CSF Label in segmentation', default="@CSF_LABEL@")
    parser.add_argument('--ACPCunit', type=str, help='ACPC unit (mm/index)', default="@ACPC_UNIT@")
    parser.add_argument('--ACPCval', type=str, help='ACPC value', default="@ACPC_VAL@")
    parser.add_argument('--LHInnerSurf', type=str, help='Left hemisphere inner surface', default="@LH_INNER@")
    parser.add_argument('--RHInnerSurf', type=str, help='Right hemisphere inner surface', default="@RH_INNER@")
    parser.add_argument('--useDfCerMask', type=str, help='Use the default cerebellum mask', default="@USE_DCM@")
    parser.add_argument('--registration', type=str, help='Perform rigid registration', default="@PERFORM_REG@")
    parser.add_argument('--skullStripping', type=str, help='Perform skull stripping', default="@PERFORM_SS@")
    parser.add_argument('--segmentation', type=str, help='Perform tissue segmentation', default="@PERFORM_TSEG@")
    parser.add_argument('--removeVentricles', type=str, help='Perform tissue segmentation', default="@PERFORM_VR@")
    parser.add_argument('--computeCSFDensity', type=str, help='Compute CSF density', default="@COMPUTE_CSFDENS@")
    parser.add_argument('--python3', type=str, help='Python3 executable path', default='@python3_PATH@')
    parser.add_argument('--ImageMath', type=str, help='ImageMath executable path', default='@ImageMath_PATH@')
    parser.add_argument('--ImageStat', type=str, help='ImageStat executable path', default='@ImageStat_PATH@')
    parser.add_argument('--ABC', type=str, help='ABC executable path', default='@ABC_CLI_PATH@')
    parser.add_argument('--output', type=str, help='Output directory', default="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)
