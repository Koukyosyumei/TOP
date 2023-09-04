VALUE_C=8

while getopts c: OPT; do
  case $OPT in
  "c")
    FLG_C="TRUE"
    VALUE_C="$OPTARG"
    ;;
  esac
done

TIMEOUT=300000

# den101d: 41 x 73: 1360 
# den201d: 37 x 37: 538
# lak202d: 182 x 159: 6240
# lak510d: 253 x 287: 7713
# orz000d: 137 x 49: 4057
# orz201d: 45 x 47: 745
# orz301d: 180 x 120: 4529

for seed in 1 2 3 4 5
 do
 for f in "den101d.map" "den201d.map" "lak202d.map" "lak510d.map" "orz000d.map" "orz201d.map" 
   do
   python3 script/random_graph_generator.py -t fgrid -g assets/${f} -s $seed -c ${VALUE_C} > "data/${f}_n=1_e=1_s=${seed}.in"
 done
done

for k in 2
 do
 for el in 1
  do
  for m in 5 10 30 50 100 200
   do    
   for h in tunnel
    do
    for f in "den101d.map" "den201d.map" "lak202d.map"
     do
     for seed in 1 2 3 4 5
      do
      ./topsolver -k ${k} -l ${el} -m ${m} -h ${h} -p random -b 100 -t ${TIMEOUT} -f output/${h}${k}${el}${m}${f}_${seed}random.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
    done
    echo ${k} ${el} ${m} ${h} & wait
   done
  done
 done
done

for k in 2
 do
 for el in 1
  do
  for m in 5 10 30 50 100 200
  do   
  for h in tunnel
   do
   for f in "lak510d.map" "orz000d.map" "orz201d.map" 
    do
    for seed in 1 2 3 4 5
     do
     ./topsolver -k ${k} -l ${el} -m ${m} -h ${h} -p random -b 100 -t ${TIMEOUT} -f output/${h}${k}${el}${m}${f}_${seed}random.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
   done
   echo ${k} ${el} ${m} ${h} & wait
  done
 done
done
done

for k in 2
 do
 for el in 1
  do
  for m in 5 10 30 50 100 200
   do    
   for h in tunnel
    do
    for f in "den101d.map" "den201d.map" "lak202d.map"
     do
     for seed in 1 2 3 4 5
      do
      ./topsolver -k ${k} -l ${el} -m ${m} -h ${h} -p clustering -b 100 -t ${TIMEOUT} -f output/${h}${k}${el}${m}${f}_${seed}cls.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
    done
    echo ${k} ${el} ${m} ${h} & wait
   done
  done
 done
done

for k in 2
 do
 for el in 1
  do
  for m in 5 10 30 50 100 200
  do   
  for h in tunnel
   do
   for f in "lak510d.map" "orz000d.map" "orz201d.map" 
    do
    for seed in 1 2 3 4 5
     do
     ./topsolver -k ${k} -l ${el} -m ${m} -h ${h} -p clustering -b 100 -t ${TIMEOUT} -f output/${h}${k}${el}${m}${f}_${seed}cls.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
   done
   echo ${k} ${el} ${m} ${h} & wait
  done
 done
done
done

for k in 2
 do
 for el in 1
  do
  for m in 5 10 30 50 100 200
   do    
   for h in tunnel
    do
    for f in "den101d.map" "den201d.map" "lak202d.map"
     do
     for seed in 1 2 3 4 5
      do
      ./topsolver -k ${k} -l ${el} -m ${m} -h ${h} -j asccost -b 100 -t ${TIMEOUT} -f output/${h}${k}${el}${m}${f}_${seed}merge.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
    done
    echo ${k} ${el} ${m} ${h} & wait
   done
  done
 done
done

for k in 2
 do
 for el in 1
  do
  for m in 5 10 30 50 100 200
  do   
  for h in tunnel
   do
   for f in "lak510d.map" "orz000d.map" "orz201d.map" 
    do
    for seed in 1 2 3 4 5
     do
     ./topsolver -k ${k} -l ${el} -m ${m} -h ${h} -j asccost -b 100 -t ${TIMEOUT} -f output/${h}${k}${el}${m}${f}_${seed}merge.out -c -u < "data/${f}_n=1_e=1_s=${seed}.in" &
     done
   done
   echo ${k} ${el} ${m} ${h} & wait
  done
 done
done
done

python3 script/report.py -d "output/*" -o report_klm_summary_${VALUE_C}.csv -t report_klm_time_${VALUE_C}.csv
