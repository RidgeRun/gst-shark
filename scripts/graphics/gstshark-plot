#! /bin/bash

# Verify if there is at least a parameter
if [ $# -lt 1 ]
then
    echo "Error: A directory name must be given"
    exit
fi

if [ ! -d $1 ]
then
    echo "Error: $1 is not a directory"
    exit
fi

tracer_list=("proctime" "interlatency" "framerate" "scheduling" "cpuusage")

#
rm -f tracer.pdf

# Loop through the tracer list
for tracer in "${tracer_list[@]}"
do
    echo "Loading ${tracer} events..."
   # Create readable file
    babeltrace $1 > datastream.log
    # Split the events in files
    grep -w ${tracer} datastream.log > ${tracer}.log
    # Get data columns
    awk '{print $1,$10,$13,$16}' ${tracer}.log > ${tracer}.mat
    # Create plots
done


echo "Loading cpuusage events..."
# Create readable file
babeltrace $1 > datastream.log
# Split the events in files
grep -w cpuusage datastream.log > cpuusage.log
# Get data columns
awk '{print $1,$10,$13,$16,$19,$22,$25,$28,$31}' cpuusage.log > cpuusage.mat

# Skip directory name
shift

# Parse options
while [[ $# -gt 0 ]]
do
    key="$1"
    case $key in
        -s|--savefig)
        SAVEFIG="--savefig"
        shift # past argument
        ;;
        -p|--persist)
        PERSIST="--persist"
        shift # past argument
        ;;
        *)
        echo "WARN: unkown \"$key\" option"
        shift
        ;;
    esac
done


# Create plots
octave -qf ${PERSIST} ./gstshark-plot.m "${tracer_list[@]}" "${SAVEFIG}"

# Remove files
for tracer in "${tracer_list[@]}"
do
    rm ${tracer}.log ${tracer}.mat -f
done


