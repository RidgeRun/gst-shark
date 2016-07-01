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

public class GstSharkCpuUsage extends TmfCommonXLineChartViewer {

	private ITmfTrace _trace = null;
	private String _event = null;
	// Minimal time displayed on plot
	private static final long MIN_TIME = 2000000000; 


	public GstSharkCpuUsage(Composite parent, String title, String xLabel, String yLabel, String event) {
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
		ITmfContext ctx = _trace.seekEvent(startTimestamp);
		ITmfEvent event = getNextFilteredByEventName(ctx, _event);

		while (null != event && 0 > event.getTimestamp().compareTo(endTimestamp)) {
			events.add(event);
			event = getNextFilteredByEventName(ctx, _event);
		}

		return events;
	}


	@Override
	protected void updateData(long start, long end, int nb, IProgressMonitor monitor) {
		List <ITmfEvent> eventsListFilterdByName;
		double x_values[];
		double refx_values[] = new double[2];
		double y_values[];
		String fieldValueName;
		ITmfTimestamp startTimestamp = _trace.createTimestamp(start);
		ITmfTimestamp endTimestamp = _trace.createTimestamp(end);

		// Filter all the event in the time range based in the name of the events
		if (end < MIN_TIME)
		{
			end = MIN_TIME;
		}
		
		eventsListFilterdByName = getEventsInRange(start, end);

		if ((null == eventsListFilterdByName) || (1 >= eventsListFilterdByName.size()))
		{
			refx_values[0] = startTimestamp.getValue() - 10000000000L*60*60*21;
			refx_values[1] = endTimestamp.getValue() - 10000000000L*60*60*21;
			setXAxis(refx_values);
			
			System.err.println("WARNING: Event list is null empty or there is only one event ");
			return;
		}
		
		// Create a list of the amount of cpus reported by event
		
		Collection<String> fieldNamesColl;
		
		fieldNamesColl = eventsListFilterdByName.get(0).getContent().getFieldNames();
		
		java.util.Iterator<String> itr =  fieldNamesColl.iterator();
		
		// For each field name, create a series of data. 
		while(itr.hasNext())
		{
			fieldValueName = itr.next();

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
