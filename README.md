## SimpleProfiler

This profiler is a sampling profiler.  
It works by creating a new thread and sample and store the main thread's EIP instruction at fixed intervals.  

When the profiler is destructed, the **SymFromAddr()** function will be called to get function name for each eip address and calculate number of samples associated with them.

## Usage

To integrate the profiler, just do the following.
1. Include **profiler.h**
2. Create an insatnce in the root function of the program (like main()) as follows.  
```c++
Profiler profiler(10000, "profiler_report.txt");  
```
The constructor has two parameters: the first is number of samples, and the second is the output file name.   
When the profiler is destructed, it will show a pie chart with default explorer.

## Report Format
The profiler report was written to a text file.  
It contains information about the percentage of samples observed in each function.  
Furthermore, a pie chart will be generated and shown at the end of the program through default browser.

If the **report_gen.sh** does not work properly, consider execute it with cygwin bash, or install newest version of win-bash.

## Notes
1. This script assumes **sh** is accessible in PATH
2. The machine should be connected to the internet when the pie chart is generated
3. If the html is opened with Internet Explorer, the content may be blocked. In this situation, click on "Allow Blocked Content" and it will run as normal.
4. The script expect to find **profiled_report.txt** for pie chart generation
5. Pie chart will be generated at the end of the program, or through running **sh report_gen.sh** after profiled_report.txt is generated.

## Others
Check out my other [portfolios](http://seanshih.com)! 

