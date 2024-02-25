'''
Need a start date and then 7 days (including start date) will be one bin type 
(rgf by default) and then the next 7 days a different bin type (wf) for as many 
week as we want.

JSON array should look like this:
[
  {
    "date": "2023-01-01",
    "bins": "wf"
  },
  {
    "date": "2023-01-04",
    "bins": "rgf"
  }
]
'''
import datetime
import json

# Arrays to hold bin date and bin types
binDate, binType = [], []

# Date to start the data from
startDate = datetime.datetime(2024, 1, 6)     #year, month, day

# Days offset used by timedelta to increase day to the next day
offset = 0

# Range == number of weeks, remember; first weeks output will be rgf
for w in range(30):
    if w % 2 == 0:
        bins = "rgf" # Even
    else:
        bins = "wf" # Odd

    # 7 days in a week :-)
    for i in range(0, 7, 1):
        now = (startDate + datetime.timedelta(days=offset)).strftime("%Y-%m-%d")
        binDate.append(now)
        binType.append(bins)
        # print("Next day is: ", today + timedelta(days=offset))

        # Increase offset to output next day
        offset = offset + 1

# Create the json array from our two arrays generated above
json_array = [{"date": day, "bins": b} for day, b in zip(binDate, binType)]
#print(json_array)

# Now output the json array to a file, no error checking
out_file = open("myfile.json", "w") 
json.dump(json_array, out_file, indent = 2) 
out_file.close() 

print('Finished, json data saved in myfile.json')
endDate = startDate + datetime.timedelta(days=offset-1)
print('  Start date: ' + startDate.strftime("%Y-%m-%d") + '   Bin type: rgf')
print('  End date:   ' + endDate.strftime("%Y-%m-%d") + '   Bin type: ' + bins)
