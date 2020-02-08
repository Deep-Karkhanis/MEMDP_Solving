
 cat $1 | egrep 'risky' | awk '{print $NF}'
 echo "----------" 
 cat $1 | egrep 'avoided risk' | awk '{print $NF}'
 echo "----------" 
 cat $1 | egrep 'give ups' | awk '{print $11}'
 echo "----------" 
 cat $1 | egrep 'give ups' | awk '{print $13}'
 echo "----------" 
 cat $1 | egrep 'give ups' | awk '{print $15}'
 echo "----------"
