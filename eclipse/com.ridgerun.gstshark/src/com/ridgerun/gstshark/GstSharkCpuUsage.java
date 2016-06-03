package com.ridgerun.gstshark;



import java.util.ArrayList;
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


	public GstSharkCpuUsage(Composite parent, String title, String xLabel, String yLabel, String event) {
		super(parent, title, xLabel, yLabel);
		_event = event;
	}

	//private List<ITmfEvent> getEventsByFieldValue (String fieldName, List<ITmfEvent> eventList ) {
		
		//System.out.println(String.format("getEventsByFieldValue: %s",fieldName));

		//List<ITmfEvent> eventListFiltered = new ArrayList<ITmfEvent>();

		//for (int event_idx = 0; event_idx  < eventList.size(); ++event_idx) {

			//if (eventList.get(event_idx).getContent().getField("padname").getValue().equals(fieldValue))
			//{
				//eventListFiltered.add(eventList.get(event_idx));
			//}
		//}
		//return eventListFiltered;
	//}


	@SuppressWarnings("unused")
	private List<String> getFieldValues (String fieldName,List<ITmfEvent> events) {
		List<String> fieldValueList = new ArrayList<String>();
		boolean value_in_list = false;
		String fieldValue;

		System.out.println("getFieldValues:");
		fieldValue = events.get(0).getContent().getField(fieldName).getValue().toString();
		System.out.println(String.format("  %s",fieldValue));
		fieldValueList.add(fieldValue);

		for (int event_idx = 1; event_idx  < events.size(); ++event_idx) {

			fieldValue = events.get(event_idx).getContent().getField(fieldName).getValue().toString();
			/* Verify if the value is already in the field values list */
			for (int field_idx = 0; field_idx < fieldValueList.size(); ++field_idx) {

				if ( 0 == fieldValueList.get(field_idx).compareTo(fieldValue))
				{
					// if
					value_in_list = true;
					break;
				}
			}
			// Add the field value if the value was not already in the list
			if (false == value_in_list)
			{
				fieldValueList.add(fieldValue);
				System.out.println(String.format("  %s",fieldValue));
			}
			value_in_list = false;
		}

		return fieldValueList;
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
		//List <ITmfEvent> eventsListFilterdByFieldValue;
		double x_values[];
		double y_values[];

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
		eventsListFilterdByName = getEventsInRange(start, end);

		System.out.print(String.format("eventsListFilterdByName.size: %d\n", eventsListFilterdByName.size()));
		
		if ((null == eventsListFilterdByName) || (0 == eventsListFilterdByName.size()))
		{
			System.out.println("WARNING: Event list is null or empty");
			return;
		}

		// Create a list of all the field values for the field name given
		//List<String> fieldValuesList = getFieldValues ("padname",eventsListFilterdByName);
		List<String> fieldValuesList = new ArrayList<String>();
		/* TODO: create the list dynamically */
		fieldValuesList.add("cpu0");
		fieldValuesList.add("cpu1");
		fieldValuesList.add("cpu2");
		fieldValuesList.add("cpu3");

		String fieldValueName;
		// For each field value, create a series of data. 
		for (int fieldValueListIdx = 0; fieldValueListIdx < fieldValuesList.size(); ++fieldValueListIdx)
		{
			fieldValueName = fieldValuesList.get(fieldValueListIdx).toString();
			System.out.print(String.format("fieldValueName: %s\n", fieldValueName));

			System.out.print(String.format("eventsListFilterdByFieldValue.size: %d\n", eventsListFilterdByName.size()));

			x_values = new double[eventsListFilterdByName.size()];
			y_values = new double[eventsListFilterdByName.size()];

			for (int i = 0; i < eventsListFilterdByName.size(); ++i) {
				/* Remove offset with 10000000000L*60*60*21 */
				x_values[i] = eventsListFilterdByName.get(i).getTimestamp().getValue() - getTimeOffset() - 10000000000L*60*60*21;
				y_values[i] = new Double(eventsListFilterdByName.get(i).getContent().getField(fieldValuesList.get(fieldValueListIdx).toString()).getValue().toString());
			}
			setXAxis(x_values);
			setSeries(fieldValuesList.get(fieldValueListIdx).toString(), y_values);
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
