VALUE_C=6

while getopts c: OPT; do
  case $OPT in
  "c")
    FLG_C="TRUE"
    VALUE_C="$OPTARG"
    ;;
  esac
done

# den101d: 41 x 73: 1360 
# den201d: 37 x 37: 538
# lak202d: 182 x 159: 6240
# lak510d: 253 x 287: 7713
# orz000d: 137 x 49: 4057
# orz201d: 45 x 47: 745
# orz301d: 180 x 120: 4529

for seed in 1 2 3 4 5
 do
 for f in "den101d.map" "den201d.map" "lak202d.map" "lak510d.map" "orz000d.map" "orz301d.map" 
   do
   python3 script/random_graph_generator.py -t fgrid -g assets/${f} -s $seed -c ${VALUE_C} > "data/${f}_n=1_e=1_s=${seed}.in"
 done
done

for k in 2 3
 do
 for el in 1 10
  do
  for h in blind tunnel
   do
   for j in "random" "asccost" "deccost" "adacost+"
    do
    for f in "den101d.map" "den201d.map" "lak202d.map" "lak510d.map" "orz000d.map" "orz301d.map" 
    do
     for seed in 1 2 3 4 5
      do
      ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 100000 -f output/${h}${j}${k}${el}${f}_${seed}merge.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
      ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 100000 -v radius -r 1 -f output/r1${h}${j}${k}${el}${f}_${seed}merge.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
      ./topsolver -k ${k} -l ${el} -h ${h} -j ${j} -b 100 -t 100000 -v radius -r 10 -f output/r10${h}${j}${k}${el}${f}_${seed}merge.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
     echo ${f} ${k} ${el} ${h} ${j} & wait
    done
   done
  done
 done
done

for k in 2 3
 do
 for el in 1 10
  do
  for h in blind tunnel
   do
   for f in "den101d.map" "den201d.map" "lak202d.map" "lak510d.map" "orz000d.map" "orz301d.map" 
    do
    for seed in 1 2 3 4 5
     do
     ./topsolver -k ${k} -l ${el} -h ${h} -p "df+" -b 100 -t 100000 -f output/${h}${k}${el}${f}_${seed}df.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     ./topsolver -k ${k} -l ${el} -h ${h} -p "df+" -b 100 -t 100000 -v radius -r 1 -f output/r1${h}${k}${el}${f}_${seed}df.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     ./topsolver -k ${k} -l ${el} -h ${h} -p "df+" -b 100 -t 100000 -v radius -r 10 -f output/r10${h}${k}${el}${f}_${seed}df.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
    echo ${f} ${k} ${el} ${h} & wait
   done
  done
 done
done

python3 script/report.py -d "output/*" -o report_summary_${VALUE_C}.csv -t report_time_${VALUE_C}.csv

