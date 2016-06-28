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

public class GstSharkBaseViewer extends TmfCommonXLineChartViewer {

	private ITmfTrace _trace = null;
	private String _event = null;


	public GstSharkBaseViewer(Composite parent, String title, String xLabel, String yLabel, String event) {
		super(parent, title, xLabel, yLabel);
		_event = event;
	}

	private List<ITmfEvent> getEventsByFieldValue (String fieldName, String fieldValue, List<ITmfEvent> eventList ) {
		List<ITmfEvent> eventListFiltered = new ArrayList<ITmfEvent>();

		for (int event_idx = 0; event_idx  < eventList.size(); ++event_idx) {

			if (eventList.get(event_idx).getContent().getField("padname").getValue().equals(fieldValue))
			{
				eventListFiltered.add(eventList.get(event_idx));
			}
		}
		return eventListFiltered;
	}


	private List<String> getFieldValues (String fieldName,List<ITmfEvent> events) {
		List<String> fieldValueList = new ArrayList<String>();
		boolean value_in_list = false;
		String fieldValue;

		fieldValue = events.get(0).getContent().getField(fieldName).getValue().toString();
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
		List <ITmfEvent> eventsListFilterdByFieldValue;
		double x_values[];
		double y_values[];

		// Filter all the event in the time range based in the name of the events
		eventsListFilterdByName = getEventsInRange(start, end);

		if ((null == eventsListFilterdByName) || (0 == eventsListFilterdByName.size()))
		{
			System.err.println("WARNING: Event list is null or empty");
			return;
		}

		// Create a list of all the field values for the field name given
		List<String> fieldValuesList = getFieldValues ("padname",eventsListFilterdByName);

		String fieldValueName;
		// For each field value, create a series of data. 
		for (int fieldValueListIdx = 0; fieldValueListIdx < fieldValuesList.size(); ++fieldValueListIdx)
		{
			fieldValueName = fieldValuesList.get(fieldValueListIdx).toString();

			// Filter all the event in the time range based in the name field value
			eventsListFilterdByFieldValue =  getEventsByFieldValue ("padname", fieldValuesList.get(fieldValueListIdx).toString(), eventsListFilterdByName );

			x_values = new double[eventsListFilterdByFieldValue.size()];
			y_values = new double[eventsListFilterdByFieldValue.size()];

			for (int i = 0; i < eventsListFilterdByFieldValue.size(); ++i) {
				
				eventsListFilterdByFieldValue.get(i).getTimestamp().toString();
				
				/* Remove offset
				 * x_values[i] = eventsListFilterdByFieldValue.get(i).getTimestamp().getValue() - getTimeOffset() - 10000000000L*60*60*21;
				 */
				x_values[i] = eventsListFilterdByFieldValue.get(i).getTimestamp().getValue() - getTimeOffset();
				y_values[i] = new Double(eventsListFilterdByFieldValue.get(i).getContent().getField("fps").getValue().toString());
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
