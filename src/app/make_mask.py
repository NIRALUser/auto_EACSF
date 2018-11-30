#!/tools/Python/Python-3.6.2/bin/python3
##	by Han Bit Yoon, Arthur Le Maout (alemaout@email.unc.edu)
#########################################################################################################  
import sys
import os
import argparse
import subprocess

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
    T1 = args.t1
    T2 = args.t2
    T2_exists=True
    if (T2 == ""):
        T2_exists=False
    atlases_dir = args.at_dir
    atlases_list = args.at_list
    ImageMath = args.ImageMath
    FSLBET = args.FSLBET
    convertITKformats = args.convertITKformats
    ANTS = args.ANTS
    WarpImageMultiTransform = args.WarpImageMultiTransform
    output_dir = args.output
    if not os.path.exists(output_dir):
        os.makedirs(output_dir)

    atlases_list = atlases_list.split(',')

    l=len(atlases_list)
    if (l%2 != 0):
        exit('Atlases list : wrong format')

    atlases_nb=int(l/2)

    ### Convert to *.nii.gz
    T1_dir = os.path.dirname(T1)
    T1_split = os.path.splitext(os.path.basename(T1))
    T1_base = T1_split[0]
    if (T1_split[1] != '.gz'):
        Input_T1_NII = os.path.join(T1_dir, "".join([T1_base,".nii.gz"]))
    else:
        T1_base = os.path.splitext(os.path.basename(T1_base))[0]
        Input_T1_NII = T1

    args_maj_vote=[ImageMath,Input_T1_NII,'-majorityVoting']

    if (T2_exists):
        T2_dir = os.path.dirname(T2)
        T2_split = os.path.splitext(os.path.basename(T2))
        T2_base = T2_split[0]
        if (T2_split[1] != '.gz'):
            Input_T2_NII = os.path.join(T2_dir, "".join([T2_base,".nii.gz"]))
        else:
            T2_base = os.path.splitext(os.path.basename(T2_base))[0]
            Input_T2_NII = T2

    if (T1 != Input_T1_NII):
        args=[convertITKformats, T1, Input_T1_NII]
        call_and_print(args)

    if (T2_exists):
        if (T2 != Input_T2_NII):
            args=[convertITKformats, T2, Input_T2_NII]
            call_and_print(args)

    T1_Only_Mask = os.path.join(T1_dir, "".join([T1_base,"_T1only"]))

    if (T2_exists):
        T2_Only_Mask = os.path.join(T2_dir, "".join([T2_base,"_T2only"]))

        ##--> T2 Jointly T1 (FSL BET)
        T2_Joint_T1_Mask = os.path.join(T1_dir, "".join([T1_base,"_T2JointT1_mask.nii.gz"]))
        T2_Joint_T1_Mask1 = os.path.join(T1_dir, "".join([T1_base,"_T2JointT1_tmp1"]))
        T2_Joint_T1_Mask2 = os.path.join(T1_dir, "".join([T1_base,"_T2JointT1_tmp2"]))

        args=[FSLBET, Input_T2_NII, T2_Joint_T1_Mask1, '-f', '0.52', '-g', '0.2', '-m', '-n', '-A2', Input_T1_NII, '-R']
        call_and_print(args)


        args=[FSLBET, Input_T2_NII, T2_Joint_T1_Mask2, '-f', '0.52', '-g', '0.2', '-m', '-n', '-A2', Input_T1_NII, '-R']
        call_and_print(args)

        T2_Joint_T1_Mask1 = ''.join([T2_Joint_T1_Mask1, "_mask.nii.gz"])
        T2_Joint_T1_Mask2 = ''.join([T2_Joint_T1_Mask2, "_mask.nii.gz"])

        args=[ImageMath, T2_Joint_T1_Mask1, '-add', T2_Joint_T1_Mask2, '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

        args=[ImageMath, T2_Joint_T1_Mask, '-threshold', '1,2', '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

        args=[ImageMath, T2_Joint_T1_Mask, '-dilate', '1,1', '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

        args=[ImageMath, T2_Joint_T1_Mask, '-erode', '1,1', '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

    Input_T1_NII_255 = os.path.join(T1_dir, "".join([T1_base,"_255.nii.gz"]))
    Pre_SkullMask = os.path.join(T1_dir, "".join([T1_base,"_255_Skull.nii.gz"]))

    args=[ImageMath, Input_T1_NII, '-rescale', '0,255', '-outfile', Input_T1_NII_255]
    call_and_print(args)

    args=[ImageMath, Input_T1_NII_255, '-threshold', '0,240', '-outfile', Pre_SkullMask]
    call_and_print(args)

    if (T2_exists):
        args=[ImageMath, T2_Joint_T1_Mask, '-mul', Pre_SkullMask, '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

        args=[ImageMath, T2_Joint_T1_Mask, '-erode', '1,1', '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

        args=[ImageMath, T2_Joint_T1_Mask, '-dilate', '1,1', '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

        args=[ImageMath, T2_Joint_T1_Mask, '-conComp', '1', '-outfile', T2_Joint_T1_Mask]
        call_and_print(args)

        for i in range(0,2):
                args=[ImageMath, T2_Joint_T1_Mask, '-dilate', '1,1', '-outfile', T2_Joint_T1_Mask]
                call_and_print(args)

                args=[ImageMath, T2_Joint_T1_Mask, '-erode', '1,1', '-outfile', T2_Joint_T1_Mask]
                call_and_print(args)

        args_maj_vote.append(T2_Joint_T1_Mask)


    for k in range(0,atlases_nb):
        atlas_base=atlases_list[2*k]
        atlas_type=int(atlases_list[2*k+1])

        if (atlas_type < 3):
            if (atlas_type == 1):
                ATLAS_suffix = '_T1.nrrd'
                IM_base = T1_base
                Input = Input_T1_NII

            else:
                ATLAS_suffix = '_T2.nrrd'
                IM_base = T2_base
                Input = Input_T2_NII

            ATLAS = os.path.join(atlases_dir,''.join([atlas_base,ATLAS_suffix]))
            ATLAS_MASK = os.path.join(atlases_dir,''.join([atlas_base,'_brainmask.nrrd']))
            OUT_MASK = os.path.join(output_dir,''.join([IM_base,'_',atlas_base,'_mask.nii.gz']))

            ANTs_MATRIX_NAME = os.path.join(output_dir,''.join([IM_base,'_',atlas_base,'_']))
            ANTs_WARP = ''.join([ANTs_MATRIX_NAME,'Warp.nii.gz'])
            ANTs_AFFINE = ''.join([ANTs_MATRIX_NAME,'Affine.txt'])

            args = [ANTS, '3', '-m', 'CC['+Input+','+ATLAS+',1,4]', '-i', '100x50x25', '-o', ANTs_MATRIX_NAME, '-t', 'SyN[0.25]', '-r', 'Gauss[3,0]']

        else:
            T1_ATLAS = os.path.join(atlases_dir,''.join([atlas_base,'_T1.nrrd']))
            T2_ATLAS = os.path.join(atlases_dir,''.join([atlas_base,'_T2.nrrd']))
            ATLAS_MASK = os.path.join(atlases_dir,''.join([atlas_base,'_brainmask.nrrd']))
            OUT_MASK = os.path.join(output_dir,''.join([T2_base,'_joined_',atlas_base,'_mask.nii.gz']))
            ANTs_MATRIX_NAME = os.path.join(output_dir, "".join([T2_base,'_joined_',atlas_base,'_']))
            ANTs_WARP_ = ''.join([ANTs_MATRIX_NAME, 'Warp.nii.gz'])
            ANTs_AFFINE = ''.join([ANTs_MATRIX_NAME, 'Affine.txt'])

            args=[ANTS, '3', '-m', 'CC['+Input_T1_NII+','+T1_ATLAS+',1,4]', '-m', 'CC['+Input_T2_NII+','+T2_ATLAS+',1,4]', '-i', '100x50x25', '-o', ANTs_MATRIX_NAME, '-t','SyN[0.25]', '-r', 'Gauss[3,0]']



        call_and_print(args)

        if (atlas_type == 2):
            args=[WarpImageMultiTransform, '3', ATLAS_MASK, OUT_MASK, ANTs_WARP, ANTs_AFFINE, '-R', Input_T2_NII, '--use-NN']
        else:
            args=[WarpImageMultiTransform, '3', ATLAS_MASK, OUT_MASK, ANTs_WARP, ANTs_AFFINE, '-R', Input_T1_NII, '--use-NN']

        call_and_print(args)

        args_maj_vote.append(OUT_MASK)



    ##--> Majority Vote
    Weighted_Majority_Mask = os.path.join(output_dir, "".join([T1_base,"_weightedMajority.nii.gz"]))
    #args=[ImageMath,Input_T1_NII,'-majorityVoting', T2_Joint_T1_Mask, T1_Only_Mask, T2_Only_Mask, COLIN_OUT_MASK, ICMB152_OUT_MASK,
    #       BIGCSF01_OUT_MASK, BIGCSF02_OUT_MASK,'-outfile', Weighted_Majority_Mask]
    args_maj_vote.extend(['-outfile',Weighted_Majority_Mask])
    call_and_print(args_maj_vote)

    TEMP_ERODE_MASK = os.path.join(output_dir, "".join([T1_base,"_TEMP_ERODE.nii.gz"]))
    args=[ImageMath,Weighted_Majority_Mask, '-erode', '8,1', '-outfile', TEMP_ERODE_MASK]
    call_and_print(args)

    FINAL_MASK = os.path.join(output_dir, "".join([T1_base,"_FinalBrainMask.nrrd"]))
    args=[ImageMath,Weighted_Majority_Mask, '-dilate', '1,1', '-outfile', Weighted_Majority_Mask]
    call_and_print(args)

    args=[ImageMath,Weighted_Majority_Mask, '-erode', '1,1', '-outfile', FINAL_MASK]
    call_and_print(args)

##############################################################################################################

if (__name__ == "__main__"):
    parser = argparse.ArgumentParser(description='Creates brain mask from template and T1&T2 images')
    parser.add_argument('--t1', nargs='?', type=str, help='T1 Image to calculate deformation field against atlas', const="@T1IMG@")
    parser.add_argument('--t2', nargs='?', type=str, help='T2 Image to calculate deformation field against atlas', const="@T2IMG@")
    parser.add_argument('--at_dir', nargs='?', type=str, help='atlases directory', const="@ATLASES_DIR@")
    parser.add_argument('--at_list', nargs='?', type=str, help='atlases list', const="@ATLASES_LIST@")
    parser.add_argument('--ImageMath', type=str, help='ImageMath executable path', default='@ImageMath_PATH@')
    parser.add_argument('--FSLBET', type=str, help='FSL BET executable path', default='@bet_PATH@')
    parser.add_argument('--convertITKformats', type=str, help='convertITKformats executable path', default='@convertITKformats_PATH@')
    parser.add_argument('--ANTS', type=str, help='ANTS executable path', default='@ANTS_PATH@')
    parser.add_argument('--WarpImageMultiTransform', type=str, help='WarpImageMultiTransform executable path', default='@WarpImageMultiTransform_PATH@')
    parser.add_argument('--output', nargs='?', type=str, help='Output directory', const="@OUTPUT_DIR@")
    args = parser.parse_args()
    main(args)
