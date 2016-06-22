package com.ridgerun.gstshark;



import java.util.ArrayList;
import java.util.Collection;
import java.util.List;

import org.eclipse.core.runtime.IProgressMonitor;
import org.eclipse.swt.widgets.Composite;
import org.eclipse.tracecompass.tmf.core.event.ITmfEvent;
import org.eclipse.tracecompass.tmf.core.timestamp.ITmfTimestamp;
import org.eclipse.tracecompass.tmf.core.trace.ITmfContext;
import org.eclipse.tracecompass.tmf.core.trace.ITmfTrace;
import org.eclipse.tracecompass.tmf.ui.viewers.xycharts.linecharts.TmfCommonXLineChartViewer;

public class GstSharkFrameRate extends TmfCommonXLineChartViewer {

	private ITmfTrace _trace = null;
	private String _event = null;
	// Minimal time displayed on plot
	private static final long MIN_TIME = 2000000000; 


	public GstSharkFrameRate(Composite parent, String title, String xLabel, String yLabel, String event) {
		super(parent, title, xLabel, yLabel);
		_event = event;
	}

	private ITmfEvent getNextFilteredByEventName(ITmfContext ctx, String name) {
		ITmfEvent event = null;

		do {
			event = _trace.getNext(ctx);
			// Avoid while verification if there is not a next event in the list
			if (event == null)
			{
				return null;
			}
	  } while (null == event || !event.getName().equals(name));

		return event;
	}


	private List<ITmfEvent> getEventsInRange (long start, long end) {
		List<ITmfEvent> events = new ArrayList<ITmfEvent>();
		ITmfTimestamp startTimestamp = _trace.createTimestamp(start);
		ITmfTimestamp endTimestamp = _trace.createTimestamp(end);
		System.out.println(String.format("Range is %s to %s", startTimestamp.toString(), endTimestamp.toString()));
		ITmfContext ctx = _trace.seekEvent(startTimestamp);
		ITmfEvent event = getNextFilteredByEventName(ctx, _event);

		while (null != event && 0 > event.getTimestamp().compareTo(endTimestamp)) {
			events.add(event);
			event = getNextFilteredByEventName(ctx, _event);
		}
		if ((null != events) && (0 < events.size() ))
		{
			System.out.println(String.format("events.size: %d", events.size()));
			System.out.println(String.format("Last ts %s - end %s", events.get(events.size()-1).getTimestamp(), endTimestamp));
		}

		return events;
	}


	@Override
	protected void updateData(long start, long end, int nb, IProgressMonitor monitor) {
		List <ITmfEvent> eventsListFilterdByName;
		double x_values[];
		double refx_values[] = new double[2];
		double refy_values[] = new double[2];
		double y_values[];
		String fieldValueName;
		ITmfTimestamp startTimestamp = _trace.createTimestamp(start);
		ITmfTimestamp endTimestamp = _trace.createTimestamp(end);

		System.out.print(String.format("Starting time: %d\n", getStartTime()));
		System.out.print(String.format("End time: %d\n", getEndTime()));
		System.out.print(String.format("W Start time: %d\n", getWindowStartTime()));
		System.out.print(String.format("W end time: %d\n", getWindowEndTime()));
		System.out.print(String.format("S start time: %d\n", getSelectionBeginTime()));
		System.out.print(String.format("S end time: %d\n", getSelectionEndTime()));

		System.out.println(start);
		System.out.println(end);
		System.out.println(nb);
		System.out.println(getTimeOffset());

		// Filter all the event in the time range based in the name of the events
		
		if (end < MIN_TIME)
		{
			end = MIN_TIME;
		}
		
		eventsListFilterdByName = getEventsInRange(start, end);

		System.out.print(String.format("eventsListFilterdByName.size: %d\n", eventsListFilterdByName.size()));
		
		if ((null == eventsListFilterdByName) || (1 >= eventsListFilterdByName.size()))
		{
			System.out.print(String.format("startTimestamp.getValue() %d %s\n",startTimestamp.getValue(),startTimestamp.toString() ));
			System.out.print(String.format("endTimestamp.getValue()   %d %s\n",endTimestamp.getValue(),endTimestamp.toString() ));
			System.out.print(String.format("getTimeOffset()           %d\n",getTimeOffset() ));
			
			refx_values[0] = startTimestamp.getValue() - 10000000000L*60*60*21;
			refx_values[1] = endTimestamp.getValue() - 10000000000L*60*60*21;
			setXAxis(refx_values);
			
			System.out.println("WARNING: Event list is null empty or there is only one event ");
			return;
		}
	
		System.out.print(String.format("eventsListFilterdByFieldValue[0] name: %s\n", eventsListFilterdByName.get(0).getName()));
		
		// Create a list of the amount of cpus reported by event
		
		Collection<String> fieldNamesColl;
		
		fieldNamesColl = eventsListFilterdByName.get(0).getContent().getFieldNames();
		
		System.out.print(String.format("fieldNamesColl.size: %d\n", fieldNamesColl.size()));
		
		java.util.Iterator<String> itr =  fieldNamesColl.iterator();
		
		// For each field name, create a series of data. 
		while(itr.hasNext())
		{
			fieldValueName = itr.next();
			System.out.print(String.format("fieldValueName: %s\n", fieldValueName));

			x_values = new double[eventsListFilterdByName.size()];
			y_values = new double[eventsListFilterdByName.size()];

			for (int i = 0; i < eventsListFilterdByName.size(); ++i) {
				/* Remove offset with 10000000000L*60*60*21 */
				x_values[i] = eventsListFilterdByName.get(i).getTimestamp().getValue() - getTimeOffset() - 10000000000L*60*60*21;
				y_values[i] = new Double(eventsListFilterdByName.get(i).getContent().getField(fieldValueName).getValue().toString());
				
			}
			setXAxis(x_values);
			setSeries(fieldValueName, y_values);
			updateDisplay();
		}
	}

	@Override
	public void loadTrace(ITmfTrace trace) {
		super.loadTrace(trace);
		_trace = trace;
	}

	@Override
	protected void createSeries() {
		ITmfContext context = _trace.seekEvent(0);
		ITmfEvent event = _trace.getNext(context);
		while (event != null) {
			if ("framerate".equals(event.getName())) {


				break;
			}
			event = _trace.getNext(context);
		}
    }
}
